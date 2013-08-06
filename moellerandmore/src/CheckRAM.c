#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "vmebus.h"

#define Verbose 1 //Set to 1 for detailed read/write information, if not to 0

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
	munmap((unsigned long *) Myaddr, 0x1000);

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

	munmap((unsigned long *) Myaddr, 0x1000);
	return 0;
}


volatile unsigned long *MypoiVUPROMRAMAdd;
volatile unsigned long MyTempPatternVUPROMAdd;

volatile unsigned long *MypoiVUPROMRAMData;
volatile unsigned long MyTempPatternVUPROMData;

volatile unsigned long *MypoiVUPROMRAMDataOut;

volatile unsigned long *VMEWriteOpen(unsigned long Myaddr, unsigned long Mypattern) {
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
	
	return Mypoi;

	if (Verbose) {
		printf("Write address = %0.8lx Pattern = %0.8lx\n", MyOrigAddr, Mypattern);
	}

//	munmap((unsigned long *) Myaddr, 0x1000);
//	return 0;
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

	if (argc != 3) {
		printf("Call this program with two parameters:  ConfigureRAM Addr.(Hex) TriggerLevel(int)\n");
		exit(1);
	}

	sscanf(argv[1], "%x\n", &BaseAddr);
	if (Verbose) {
		printf("Base address: %0.8lx\n",BaseAddr);
	}


	int TriggerLevel;
	sscanf(argv[2], "%i\n", &TriggerLevel);
	if ( (TriggerLevel == 1) || (TriggerLevel == 2) ) {
		printf("TriggerLevel: L%i\n",TriggerLevel);
	} else {
		printf("TriggerLevel needs to be 1 or 2. Exit.\n");
		return -1;
	}


	OpenVMEbus();

	SetTopBits(BaseAddr);

	if (Verbose) {printf("Start: \n");}

	int WriteHistoBinAddr;
	unsigned long LastReadData;

	FILE *fr;
	char line[80];
	unsigned long x;

	//Write into RAM
	if (TriggerLevel == 1) {
		fr = fopen("TriggerRAML1.dat", "rt");  /* open the file for reading, "rt" means open the file for reading text */
	} else {
		fr = fopen("TriggerRAML2.dat", "rt");  /* open the file for reading, "rt" means open the file for reading text */
	}

	if (!fr) {printf("Could not open file. Is file TriggerRAML1.dat or TriggerRAML2.dat present in current directory?\n"); return -1;}

	i = -1;
	int TopUpToMatchTriggerLevel = 0;
	if (TriggerLevel == 2) {
		TopUpToMatchTriggerLevel = 0x40;
	}

	VMEWrite(BaseAddr+0x2200+TopUpToMatchTriggerLevel,1); //Write Mode
	MypoiVUPROMRAMAdd = VMEWriteOpen(BaseAddr+0x2210+TopUpToMatchTriggerLevel,0); //RAM Address
	MypoiVUPROMRAMData = VMEWriteOpen(BaseAddr+0x2220+TopUpToMatchTriggerLevel,0); //Data Address
	MypoiVUPROMRAMDataOut = VMEWriteOpen(BaseAddr+0x2230+TopUpToMatchTriggerLevel,0); //DataOut
	while(fgets(line, 80, fr) != NULL) {
		i++;
		/* get a line, up to 80 chars from fr.  done if NULL */
		sscanf (line, "%i", &x);

	}
	if (i != 65535) {
		printf("Error: Written %i data words. Should be 65535. Error in input file. \n\n", i);
	}
	fclose(fr);  /* close the file prior to exiting the routine */
	VMEWrite(BaseAddr+0x2200+TopUpToMatchTriggerLevel,0);  //Back to read mode

	// **************************************************************************

	//Read from RAM
	printf("Compare content in RAM now.\n");
	if (TriggerLevel == 1) {
		fr = fopen ("TriggerRAML1.dat", "rt");  /* open the file for reading, "rt" means open the file for reading text */
	} else {
		fr = fopen ("TriggerRAML2.dat", "rt");  /* open the file for reading, "rt" means open the file for reading text */
	}
	i = -1;
	int TempError = 0;
	unsigned long tempX;	

	while(fgets(line, 80, fr) != NULL) {
		i++;
		/* get a line, up to 80 chars from fr.  done if NULL */
		sscanf (line, "%i", &x);
		/* convert the string to a long int */
		//VMEWrite(BaseAddr+0x2210+TopUpToMatchTriggerLevel,i); //RAM Address
//		MyTempPatternVUPROMAdd = *MypoiVUPROMRAMAdd;  //Erst mal die Adresse auf dem VMEbus setzen durch Lesen, um einen Fehler im VUPROM module auszugleichen
		*MypoiVUPROMRAMAdd = i;


		//tempX = VMERead(BaseAddr+0x2230+TopUpToMatchTriggerLevel); //Data Address
//		tempX  = *MypoiVUPROMRAMDataOut ;  //Erst mal die Adresse auf dem VMEbus setzen durch Lesen, um einen Fehler im VUPROM module auszugleichen
		tempX  = *MypoiVUPROMRAMDataOut ;  //Erst mal die Adresse auf dem VMEbus setzen durch Lesen, um einen Fehler im VUPROM module auszugleichen
		
		if (x != tempX) {
			if (Verbose) {printf("\nAddress: %i \t Data: %x\n", i,x);}
			if (Verbose) {printf("Should be: %i\t it is: %i\n",x,tempX);}
			TempError++;
		};

		if (Verbose==0) {
			if (i%10000 == 0) {
				printf("Address: %i\n",i);
			}
		}
	}
	if (i != 65535) {
		printf("Error: Checked %i data words. Should be 65535. Error in input file. \n\n", i);
	}
	fclose(fr);  /* close the file prior to exiting the routine */

	if (TempError) {
		printf("Error. %i errors.\n",TempError);
	} else {
		printf("All okay.\n");
	}
	
	
	printf("Comparision finised.\n");

}
