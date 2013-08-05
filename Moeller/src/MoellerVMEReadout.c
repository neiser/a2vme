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
	Mypattern = *Mypoi; //Doppelt lesen

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

	MyTempPattern = *Mypoi;  //Erst mal die Adresse auf dem VMEbus setzen durch Lesen, um einen Fehler im VUPROM module auszugleichen
	*Mypoi = Mypattern;  //Doppelter Zugriff beim Schreiben

	if (Verbose) {
		printf("Write address = %0.8lx Pattern = %0.8lx\n", MyOrigAddr, Mypattern);
	}

	return 0;
}

unsigned long SetTopBits(unsigned long Myaddr) {
	volatile unsigned long *Mypoi;

	// obere 3 Bits setzen via Register 0xaa000000
    if ((Mypoi = vmebus(0xaa000000, 0x1000)) == NULL) {
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
	int i, k, helicity;
	unsigned long BaseAddr;

	if (argc != 2) {
		printf("Aufruf = MoellerVMEReadout Addr. (Hex)\n");
		exit(1);
	}

	sscanf(argv[1], "%x\n", &BaseAddr);
	if (Verbose) {
		printf("Baseadresse: %0.8lx\n",BaseAddr);
	}

	OpenVMEbus();

	SetTopBits(BaseAddr);

	if (Verbose) {printf("Start: \n");}

	int WriteHistoBinAddr;
	for (i=0;i<=255;i++) {
		WriteHistoBinAddr = (i+128)%256;
		if (Verbose) {printf("\nBin number%i \n", WriteHistoBinAddr);}
		for (helicity = 0; helicity <= 1; helicity++) {
			VMEWrite(BaseAddr+0x2c10,helicity*0x100+WriteHistoBinAddr); //BASE_TRIG_HistogramRAM_AddrB = 0
			for (k = 1; k<=NumberOfLeftChannels*NumberOfPairsPerLeftCh; k++) {
				printf("%lu\t", VMERead(BaseAddr+0x2800+k*4));
			}
		}
		printf("\n");
	}
}
