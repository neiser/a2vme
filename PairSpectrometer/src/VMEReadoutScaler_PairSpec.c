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
	if ((Mypoi = vmebus(0, 0xaa000000, 0x1000)) == NULL) {
		perror("Error opening device.\n");
		exit (-1);
	}
	*Mypoi = Myaddr & 0xe0000000;
	// obere 3 Bits setzen: fertig

	return 0;
}

void MyReadOut(int Module, int SubModule, int ConnectorNr) {
	//Module = 1,2,3,4
	//SubModule = 1,3,6 (Delayed, Undelayed, open
	//ConnectorNr = 0,1,2,3 (IN1 IN2, INOUT1, INOUT3) 
	int k;
	for (k = 32*ConnectorNr; k<32*(ConnectorNr+1); k++) { printf("%lu\n", VMERead(0x1000000*Module +0x1000*SubModule +k*4)); }
}

int main(argc, argv)
	int argc; char *argv[];
{
	int i;
	unsigned long BaseAddr = 0x1000000;

	OpenVMEbus();

	SetTopBits(BaseAddr);

	if (Verbose) {printf("Start: \n");}
	
	//Save all counters
	for (i=1;i<=4;i++) {
		VMEWrite(BaseAddr*i+0x1804,1); //Save Counter
		VMEWrite(BaseAddr*i+0x3804,1); //Save Counter
		VMEWrite(BaseAddr*i+0x6804,1); //Save Counter
	}

	//Delayed
	MyReadOut(1,1,0); //AB
	MyReadOut(1,1,1); //CD
	MyReadOut(2,1,0); //EF
	MyReadOut(3,1,0); //GH
	MyReadOut(4,1,0); //IJ
	MyReadOut(2,1,1); //KL
	MyReadOut(3,1,1); //MN
	MyReadOut(4,1,2); //OP
	MyReadOut(3,1,2); //QR
	MyReadOut(2,1,2); //ST
	MyReadOut(1,1,2); //UV
	MyReadOut(1,1,3); //INOUT3 x01
	MyReadOut(2,1,3); //INOUT3 x02
	MyReadOut(3,1,3); //INOUT3 x03
	MyReadOut(4,1,3); //INOUT3 x04


	//Undelayed
	MyReadOut(1,3,0); //AB
	MyReadOut(1,3,1); //CD
	MyReadOut(2,3,0); //EF
	MyReadOut(3,3,0); //GH
	MyReadOut(4,3,0); //IJ
	MyReadOut(2,3,1); //KL
	MyReadOut(3,3,1); //MN
	MyReadOut(4,3,2); //OP
	MyReadOut(3,3,2); //QR
	MyReadOut(2,3,2); //ST
	MyReadOut(1,3,2); //UV
	MyReadOut(1,3,3); //INOUT3 x01
	MyReadOut(2,3,3); //INOUT3 x02
	MyReadOut(3,3,3); //INOUT3 x03
	MyReadOut(4,3,3); //INOUT3 x04

	//open
	MyReadOut(1,6,0); //AB
	MyReadOut(1,6,1); //CD
	MyReadOut(2,6,0); //EF
	MyReadOut(3,6,0); //GH
	MyReadOut(4,6,0); //IJ
	MyReadOut(2,6,1); //KL
	MyReadOut(3,6,1); //MN
	MyReadOut(4,6,2); //OP
	MyReadOut(3,6,2); //QR
	MyReadOut(2,6,2); //ST
	MyReadOut(1,6,2); //UV
	MyReadOut(1,6,3); //INOUT3 x01
	MyReadOut(2,6,3); //INOUT3 x02
	MyReadOut(3,6,3); //INOUT3 x03
	MyReadOut(4,6,3); //INOUT3 x04

}
