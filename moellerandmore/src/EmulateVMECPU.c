#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "vmebus.h"

#define Verbose 0 //Set to 1 for detailed read/write information, if not to 0

volatile unsigned long *AllocateVMEAccess(unsigned long Myaddr) {
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
	//Mypattern = *Mypoi; //Doppelt lesen

	return Mypoi;
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

//	MyTempPattern = *Mypoi;  //Erst mal die Adresse auf dem VMEbus setzen durch Lesen, um einen Fehler im VUPROM module auszugleichen
	*Mypoi = Mypattern;  //Doppelter Zugriff beim Schreiben

	if (Verbose) {
		printf("Write address = %0.8lx Pattern = %0.8lx\n", MyOrigAddr, Mypattern);
	}

	munmap((unsigned long *) Mypoi, 0x1000);
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

	munmap((unsigned long *) Mypoi, 0x1000);
	return 0;
}

int main(argc, argv)
	int argc; char *argv[];
{
	int i, k, l;
	unsigned long BaseAddr;

	if (argc != 2) {
		printf("Call this program with two parameters:  EmulateCPU Addr.(Hex)\n");
		exit(1);
	}

	sscanf(argv[1], "%x\n", &BaseAddr);
	if (Verbose) {
		printf("Base address: %0.8lx\n",BaseAddr);
	}


	OpenVMEbus();

	SetTopBits(BaseAddr);

	if (Verbose) {printf("Start: \n");}

	volatile unsigned long *Mypoi;
	Mypoi = AllocateVMEAccess(BaseAddr+0x2000);

	int Counter = 0;
	int Counter2 = 0;
	while (-1) {
		Counter2++;
		if (*(Mypoi+0x460/4) != 0) {
			Counter++;
	//		printf("%x\n",*(Mypoi+0x2a0/4));
			if ((Counter % 10000) == 0) {
				printf("%i\t%i\t%f\n",Counter,Counter2,Counter*1.0/Counter2);
			}
			if ((Counter % 10000) == 0) {
				*(Mypoi+0x470/4) = 2; //Stop
				printf("TCS reset...");
				//sleep(1);
				printf(" done.\n");
				*(Mypoi+0x470/4) = 1; //Start
			}

			*(Mypoi+0x450/4) = 1; //CPU ACK
		}
	}
	
}

