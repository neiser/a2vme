#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "vmebus.h"

#define Verbose 0 //Set to 1 for detailed read/write information, if not to 0
#define NumberOfLeftChannels 10
#define NumberOfPairsPerLeftCh 5


unsigned long VMERead(unsigned long Myaddr) {
	unsigned long Myrest, MyOrigAddr, Mypattern;
	volatile unsigned long *Mypoi;
	MyOrigAddr = Myaddr;
	Myaddr &= 0x1fffffff;	// obere Bits ausmaskieren
	Myrest = Myaddr % 0x1000;
	Myaddr = (Myaddr / 0x1000) * 0x1000;

	if ((Mypoi = vmeext(Myaddr, 0x1000)) == NULL) {
		perror("Error opening device.\n");
		exit (-1);
	}
	Mypoi += (Myrest / 4);  //da 32Bit-Zugriff

	//Lesen
	Mypattern = *Mypoi;
	if (Verbose) {
		printf("Read address = %0.8lx Pattern = %0.8lx\n", MyOrigAddr, Mypattern);
	}

	return Mypattern;
}

unsigned long VMEWrite(unsigned long Myaddr, unsigned long Mypattern) {
	unsigned long MyOrigAddr, Myrest;
	volatile unsigned long *Mypoi;
	volatile unsigned long MyTempPattern;
	MyOrigAddr = Myaddr;
	Myaddr &= 0x1fffffff;	// obere Bits ausmaskieren
	Myrest = Myaddr % 0x1000;
	Myaddr = (Myaddr / 0x1000) * 0x1000;

	if ((Mypoi = vmeext(Myaddr, 0x1000)) == NULL) {
		perror("Error opening device.\n");
		exit (-1);
	}
	Mypoi += (Myrest / 4);  //da 32Bit-Zugriff

	*Mypoi = Mypattern;  //Doppelter Zugriff beim Schreiben

	if (Verbose) {
		printf("Write address = %0.8lx Pattern = %0.8lx\n", MyOrigAddr, Mypattern);
	}

	return 0;
}

unsigned long SetTopBits(unsigned long Myaddr) {
	volatile unsigned long *Mypoi;

	// obere 3 Bits setzen via Register 0xaa000000
	if ((Mypoi = vmebus(0, 0xaa000000, 0x1000)) == NULL) {
		perror("Error opening device.\n");
		exit (-1);
	}
	*Mypoi = Myaddr & 0xe0000000;
	// obere 3 Bits setzen: fertig

	return 0;
}

int main(argc, argv)
	int argc; char *argv[];
{
	int i, k;
	unsigned long BaseAddr;

	BaseAddr = 0x1000000;

	OpenVMEbus();

	SetTopBits(BaseAddr);

	if (Verbose) {printf("Start: \n");}

	//Save Counters
	for (i=1;i<=10;i++) {
		BaseAddr = 0x1000000 * i;
		VMEWrite(BaseAddr+0x1804,1); //Save Counter
	}

	//BaF2
	for (i=1;i<=6;i++) {
		BaseAddr = 0x1000000 * i;
		for (k = 0; k<=32*7-1; k++) {
			printf("%lu\n", VMERead(BaseAddr+0x1000+k*4));
		}
	}

	//Vetos
	for (i=7;i<=9;i++) {
		BaseAddr = 0x1000000 * i;
		for (k = 0; k<=32*4-1; k++) {
			printf("%lu\n", VMERead(BaseAddr+0x1000+k*4));
		}
	}

	//Master
	for (i=10;i<=10;i++) {
		BaseAddr = 0x1000000 * i;
		for (k = 0; k<=32*1-1; k++) {
			printf("%lu\n", VMERead(BaseAddr+0x1000+k*4));
		}
	}

	//PbWO
	for (i=7;i<=9;i++) {
		BaseAddr = 0x1000000 * i;
		for (k = 32*4; k<=32*5-1; k++) {
			printf("%lu\n", VMERead(BaseAddr+0x1000+k*4));
		}
	}

}
