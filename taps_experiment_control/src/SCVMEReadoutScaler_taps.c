#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "vmebus.h"
#include <string.h>

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
	int i, k;
	unsigned long BaseAddr;

	BaseAddr = 0x1000000;

	OpenVMEbus();

	SetTopBits(BaseAddr);

	if (Verbose) {printf("Start: \n");}

	//Clear Counters
	for (i=10;i<=10;i++) {
		BaseAddr = 0x1000000 * i;
		VMEWrite(BaseAddr+0x3800,1); //Save Counter
	}

	sleep(1);

	//Save Counters
	for (i=10;i<=10;i++) {
		BaseAddr = 0x1000000 * i;
		VMEWrite(BaseAddr+0x3804,1); //Save Counter
	}

	unsigned long Values[(32*2)];

	//Master
	for (i=10;i<=10;i++) {
		BaseAddr = 0x1000000 * i;
		for (k = 0; k<=32*2-1; k++) {
			Values[k] = VMERead(BaseAddr+0x3000+k*4);
			//printf("%lu\n", VMERead(BaseAddr+0x3000+k*4));
		}
	}

	char buffer[1000];
	char str2[40];

	//Node 0 = TAPS
    sprintf(buffer, "wget -O httpresult.txt \"http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=0&");
	for (i=0;i<32;i++) {
		sprintf(str2, "Value%i=%u&", i, Values[i]);
		strcat(buffer, str2);
 	}
	strcat(buffer, "\"");
	//printf("%s\n", buffer);
	system(buffer);

	//Node 1 = CB
    sprintf(buffer, "wget -O httpresult.txt \"http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=1&");
	for (i=0;i<32;i++) {
		sprintf(str2, "Value%i=%u&", i, Values[i+32]);
		strcat(buffer, str2);
 	}
	strcat(buffer, "\"");
	//printf("%s\n", buffer);
	system(buffer);



    //system("wget -O httpresult.txt \"http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=0&Value0=1.5&Value1=7\"");
}
