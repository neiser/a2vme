/*

TAPS Scaler program
author: Tigran Rostomyan, Peter-Bernd Otte
last update: 25.10.2011


*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "vmebus.h"

#define Verbose 0 //Set to 1 for detailed read/write information, if not to 0


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

int main(argc, argv)
	int argc; char *argv[];
{
	int i, k;
	unsigned long BaseAddr;
	
	BaseAddr = 0x1000000;

	OpenVMEbus();

	SetTopBits(BaseAddr);

	while (1) {
		system("clear");

		BaseAddr = 0x1000000 * 10;

		//Save Counters
		VMEWrite(BaseAddr+0x1804,1); //Save Counter

		//Master
		printf("\n");
		if (VMERead(BaseAddr+0x2400) == 1) {
			printf("             TAPS in COUPLED MODE\n");
		} else {
			printf("             TAPS in STANDALONE MODE\n");
		}
			
		printf("\n");

		printf("  CFD        F %9lu  %9lu    E\n", VMERead(BaseAddr+0x1000+(32+5)*4), VMERead(BaseAddr+0x1000+(32+4)*4));
		printf("          A %9lu        %9lu    D\n", VMERead(BaseAddr+0x1000+(32+0)*4), VMERead(BaseAddr+0x1000+(32+3)*4));
		printf("             B %9lu  %9lu    C\n", VMERead(BaseAddr+0x1000+(32+1)*4), VMERead(BaseAddr+0x1000+(32+2)*4));
			
		printf("\n\n");
			
		printf("  LED1       F %9lu  %9lu    E\n", VMERead(BaseAddr+0x1000+(32+5+6*1)*4), VMERead(BaseAddr+0x1000+(32+4+6*1)*4));
		printf("          A %9lu        %9lu    D\n", VMERead(BaseAddr+0x1000+(32+0+6*1)*4), VMERead(BaseAddr+0x1000+(32+3+6*1)*4));
		printf("             B %9lu  %9lu    C\n", VMERead(BaseAddr+0x1000+(32+1+6*1)*4), VMERead(BaseAddr+0x1000+(32+2+6*1)*4));			

		printf("\n\n");
			
		printf("  LED2       F %9lu  %9lu    E\n", VMERead(BaseAddr+0x1000+(32+5+6*2)*4), VMERead(BaseAddr+0x1000+(32+4+6*2)*4));
		printf("          A %9lu        %9lu    D\n", VMERead(BaseAddr+0x1000+(32+0+6*2)*4), VMERead(BaseAddr+0x1000+(32+3+6*2)*4));
		printf("             B %9lu  %9lu    C\n", VMERead(BaseAddr+0x1000+(32+1+6*2)*4), VMERead(BaseAddr+0x1000+(32+2+6*2)*4));

		printf("\n\n");
			
		printf("  LED VETO   F %9lu  %9lu    E\n", VMERead(BaseAddr+0x1000+(32+5+6*3)*4), VMERead(BaseAddr+0x1000+(32+4+6*3)*4));
		printf("          A %9lu        %9lu    D\n", VMERead(BaseAddr+0x1000+(32+0+6*3)*4), VMERead(BaseAddr+0x1000+(32+3+6*3)*4));
		printf("             B %9lu  %9lu    C\n", VMERead(BaseAddr+0x1000+(32+1+6*3)*4), VMERead(BaseAddr+0x1000+(32+2+6*3)*4));

		printf("\n");
			
		printf("      OR                         M2+\n");
		printf("CFD  OR[%x]: %9lu(%9lu)  [%x]: %9lu(%9lu)\n",VMERead(BaseAddr+0x2100),VMERead(BaseAddr+0x1000+9*4),VMERead(BaseAddr+0x1000+2*4), VMERead(BaseAddr+0x2140),VMERead(BaseAddr+0x1000+12*4),VMERead(BaseAddr+0x1000+5*4));
		printf("LED1 [%x]: %9lu(%9lu)  [%x]: %9lu(%9lu)\n",VMERead(BaseAddr+0x2110),VMERead(BaseAddr+0x1000+10*4),VMERead(BaseAddr+0x1000+3*4),VMERead(BaseAddr+0x2150),VMERead(BaseAddr+0x1000+13*4),VMERead(BaseAddr+0x1000+6*4));
		printf("LED2 [%x]: %9lu(%9lu)  [%x]: %9lu(%9lu)\n",VMERead(BaseAddr+0x2120),VMERead(BaseAddr+0x1000+11*4),VMERead(BaseAddr+0x1000+4*4),VMERead(BaseAddr+0x2160),VMERead(BaseAddr+0x1000+14*4),VMERead(BaseAddr+0x1000+7*4));
		printf("LEDV [%x]: %9lu(%9lu)  [%x]: %9lu(%9lu)\n",VMERead(BaseAddr+0x2130),VMERead(BaseAddr+0x1000+23*4),VMERead(BaseAddr+0x1000+21*4),VMERead(BaseAddr+0x2170),VMERead(BaseAddr+0x1000+24*4),VMERead(BaseAddr+0x1000+22*4));
		printf("\n");
		printf("Pulser [%x]  : %9lu     TAPS L1 Trigger  : %9lu\n",VMERead(BaseAddr+0x2180),VMERead(BaseAddr+0x1000+15*4),VMERead(BaseAddr+0x1000+16*4));
		printf("TAPS L1 Int : %9lu     Accepted to TAPS : %9lu\n",VMERead(BaseAddr+0x1000+17*4),VMERead(BaseAddr+0x1000+18*4));
		printf("CB coupled triggers: %9lu     TAPS busy : %9lu\n",VMERead(BaseAddr+0x1000+1*4),VMERead(BaseAddr+0x1000+0*4));
		printf("TAPS M2+: %9lu\n",VMERead(BaseAddr+0x1000+7*4));
		
		printf("\n\nHints: All values in Hz. Rates in ( ) are not prescaled. [] = prescaler setting\n");
		printf("       End with CTRL+C.\n");
		
		VMEWrite(BaseAddr+0x1800,1); //Clear Counter
		sleep(1);
	};
}
