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
	for (i=5;i<=5;i++) {
		BaseAddr = 0x1000000 * i;
		VMEWrite(BaseAddr+0x1800,1); //Save Counter
	}

	sleep(1);

	//Save Counters
	for (i=5;i<=5;i++) {
		BaseAddr = 0x1000000 * i;
		VMEWrite(BaseAddr+0x1804,1); //Save Counter
	}

	unsigned long Values[32*2];

	//Master
	BaseAddr = 0x1000000 * 5;
	Values[0] = VMERead(BaseAddr+0x1000+12*4); //16OR Tagger Section A
	Values[1] = VMERead(BaseAddr+0x1000+13*4); //16OR Tagger Section B
	Values[2] = VMERead(BaseAddr+0x1000+14*4); //16OR Tagger Section C
	Values[3] = VMERead(BaseAddr+0x1000+15*4); //16OR Tagger Section D
	Values[4] = VMERead(BaseAddr+0x1000+44*4); //16OR Tagger Section E
	Values[5] = VMERead(BaseAddr+0x1000+45*4); //16OR Tagger Section F
	Values[6] = VMERead(BaseAddr+0x1000+76*4); //16OR Tagger Section G
	Values[7] = VMERead(BaseAddr+0x1000+77*4); //16OR Tagger Section H
	Values[8] = VMERead(BaseAddr+0x1000+108*4); //16OR Tagger Section I
	Values[9] = VMERead(BaseAddr+0x1000+109*4); //16OR Tagger Section J
	Values[10] = VMERead(BaseAddr+0x1000+46*4); //16OR Tagger Section K
	Values[11] = VMERead(BaseAddr+0x1000+47*4); //16OR Tagger Section L
	Values[12] = VMERead(BaseAddr+0x1000+78*4); //16OR Tagger Section M
	Values[13] = VMERead(BaseAddr+0x1000+79*4); //16OR Tagger Section N
	Values[14] = VMERead(BaseAddr+0x1000+112*4); //16OR Tagger Section O
	Values[15] = VMERead(BaseAddr+0x1000+113*4); //16OR Tagger Section P
	Values[16] = VMERead(BaseAddr+0x1000+80*4); //16OR Tagger Section Q
	Values[17] = VMERead(BaseAddr+0x1000+81*4); //16OR Tagger Section R
	Values[18] = VMERead(BaseAddr+0x1000+48*4); //16OR Tagger Section S
	Values[19] = VMERead(BaseAddr+0x1000+49*4); //16OR Tagger Section T
	Values[20] = VMERead(BaseAddr+0x1000+16*4); //16OR Tagger Section U
	Values[21] = VMERead(BaseAddr+0x1000+17*4); //16OR Tagger Section V
	
	Values[22] = VMERead(BaseAddr+0x1000+8*4); //prescaled clocked mami response (+)
	Values[23] = VMERead(BaseAddr+0x1000+9*4); //prescaled clocked output from generator (+)
	Values[24] = VMERead(BaseAddr+0x1000+10*4); //prescaled clocked inverted output from generator (-)
	Values[25] = VMERead(BaseAddr+0x1000+11*4); //prescaled clocked inhibit, if set, status of source is indetermined

	Values[26] = VMERead(BaseAddr+0x1000+29*4); //pairSpek
	Values[27] = VMERead(BaseAddr+0x1000+31*4); //pairSpek delayed streched

	Values[28] = VMERead(BaseAddr+0x1000+138*4); //IonP2 Chamber
	Values[29] = VMERead(BaseAddr+0x1000+139*4); //Farraday

	Values[30] = 0;
	Values[31] = 0;

	//////////////////////////////////////////////////////////////////////////
	Values[32] = VMERead(BaseAddr+0x1000+128*4); //SlowControl Tagger, Scaler In 0
	Values[33] = VMERead(BaseAddr+0x1000+129*4); //SlowControl Tagger, Scaler In 1
	Values[34] = VMERead(BaseAddr+0x1000+130*4); //SlowControl Tagger, Scaler In 2
	Values[35] = VMERead(BaseAddr+0x1000+131*4); //SlowControl Tagger, Scaler In 3
	Values[36] = VMERead(BaseAddr+0x1000+132*4); //SlowControl Tagger, Scaler In 4
	Values[37] = VMERead(BaseAddr+0x1000+133*4); //SlowControl Tagger, Scaler In 5
	Values[38] = VMERead(BaseAddr+0x1000+134*4); //SlowControl Tagger, Scaler In 6
	Values[39] = VMERead(BaseAddr+0x1000+135*4); //SlowControl Tagger, Scaler In 7
	Values[40] = VMERead(BaseAddr+0x1000+136*4); //SlowControl Tagger, Scaler In 8
	Values[41] = VMERead(BaseAddr+0x1000+137*4); //SlowControl Tagger, Scaler In 9
	Values[42] = VMERead(BaseAddr+0x1000+138*4); //IonP2 Chamber, SlowControl Tagger
	Values[43] = VMERead(BaseAddr+0x1000+139*4); //Farraday, SlowControl Tagger
	Values[44] = VMERead(BaseAddr+0x1000+140*4); //SlowControl Tagger
	Values[45] = VMERead(BaseAddr+0x1000+141*4); //SlowControl Tagger
	Values[46] = VMERead(BaseAddr+0x1000+142*4); //SlowControl Tagger
	for (i=47;i<(32*2);i++) {
		Values[i] = 0;
	}

	//////////////////////////////////////////////////////////////////////////



	char buffer[1000];
	char str2[40];

	//Node 2 = Tagger Part 1
	sprintf(buffer, "fetch -o httpresult.txt \"http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=2&"); 
	for (i=0;i<32;i++) {
		sprintf(str2, "Value%i=%u&", i, Values[i]);
		strcat(buffer, str2);
 	}
	strcat(buffer, "\"");
	printf("%s\n", buffer);
	system(buffer);


	//Node 2 = Tagger Part 2
	sprintf(buffer, "fetch -o httpresult.txt \"http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=3&");
	for (i=0;i<32;i++) {
		sprintf(str2, "Value%i=%u&", i, Values[i+32]);
		strcat(buffer, str2);
 	}
	strcat(buffer, "\"");
	printf("%s\n", buffer);
	system(buffer);



	//system("fetch -o httpresult.txt \"http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=0&Value0=1.5&Value1=7\"");
}
