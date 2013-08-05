/*

TAPS Scaler program
author: Tigran Armand Rostomyan, Peter-Bernd Otte
last update: 10.Oct.2012

*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "vmebus.h"

#define Verbose 0 //Set to 1 for detailed read/write information, if not to 0


unsigned long *OpenVMEBusForReadWrite(unsigned long Myaddr) {
	unsigned long Myrest;
	unsigned long *Mypoi;

	Myaddr &= 0x1fffffff;	// obere Bits ausmaskieren
	Myaddr = (Myaddr / 0x10000) * 0x10000;

	if ((Mypoi = vmeext(Myaddr, 0x10000)) == NULL) {
		perror("Error opening device.\n");
		exit (-1);
	}

	Myrest = Myaddr % 0x10000;
	//Mypoi += (Myrest / 4);  //da 32Bit-Zugriff

	return Mypoi;
}
int CloseVMEBusForReadWrite(unsigned long * VMEBusAccessMypoi) {
	munmap(VMEBusAccessMypoi, 0x10000);

	return 0;
}

unsigned long NewVMERead(unsigned long *MyVMEPointer, unsigned long MyAddrRest) {
	unsigned long Mypattern;
	volatile unsigned long *Mypoi;

	Mypoi = MyVMEPointer + (MyAddrRest / 4);  //da 32Bit-Zugriff

	//Lesen
	Mypattern = *Mypoi;
//	Mypattern = *Mypoi; //Doppelt lesen

	return Mypattern;
}

unsigned long NewVMEWrite(unsigned long *MyVMEPointer, unsigned long MyAddrRest, unsigned long Mypattern) {
	unsigned long MyTemppattern;
	volatile unsigned long *Mypoi;

	Mypoi = MyVMEPointer + (MyAddrRest / 4);  //da 32Bit-Zugriff

	//Lesen
//	MyTemppattern = *Mypoi; //First read the address to compensate for a problem in the VME module
	*Mypoi = Mypattern; //now read

	return Mypattern;
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

	munmap((unsigned long *) Mypoi, 0x1000);

	return 0;
}

int main(argc, argv)
	int argc; char *argv[];
{
	int i, k;
	int FirmwaresOkay = 1;
	unsigned long BaseAddr;
	unsigned long ActualPulserValue; //To test, whether some other program (running in parallel) resets the scalers
	unsigned long * VMEBusAccessMypoi[11];
	
	BaseAddr = 0x1000000 * 10;

	OpenVMEbus();

	SetTopBits(BaseAddr);

	//Open VMEbus in RAM for all 10 cards
	for (i=1;i<=10;i++) {
		VMEBusAccessMypoi[i] = OpenVMEBusForReadWrite(0x1000000 * i);
	}


	NewVMEWrite(VMEBusAccessMypoi[10],0x1800,1); //Clear Counter
	printf("Initialising, please wait...\n");
	for (i=1;i<=6;i++)   { printf("  Firmware checked: Slave,  VME address 0x%0.2lx has 0x%0.8lx. ",i,NewVMERead(VMEBusAccessMypoi[i],0x2f00)); if (NewVMERead(VMEBusAccessMypoi[i],0x2f00) != 0x12112003) {printf("Failed.\n"); FirmwaresOkay = 0;} else {printf("Passed.\n"); } }
	for (i=7;i<=9;i++)   { printf("  Firmware checked: Veto,   VME address 0x%0.2lx has 0x%0.8lx. ",i,NewVMERead(VMEBusAccessMypoi[i],0x2f00)); if (NewVMERead(VMEBusAccessMypoi[i],0x2f00) != 0x12112103) {printf("Failed.\n"); FirmwaresOkay = 0;} else {printf("Passed.\n"); } }
	for (i=10;i<=10;i++) { printf("  Firmware checked: Master, VME address 0x%0.2lx has 0x%0.8lx. ",i,NewVMERead(VMEBusAccessMypoi[i],0x2f00)); if (NewVMERead(VMEBusAccessMypoi[i],0x2f00) != 0x12120505) {printf("Failed.\n"); FirmwaresOkay = 0;} else {printf("Passed.\n"); } }
	if (FirmwaresOkay == 0) {
		printf("At least one TAPS VUPROM has a wrong firmware version. Exiting.\n");
		exit(-1);
	} else {
		printf("All TAPS VUPROMs have correct firmware versions.\n");
	};

	int MyLoop = 1;
	while (MyLoop) {
		if (argc > 2) {MyLoop = 0;}

		NewVMEWrite(VMEBusAccessMypoi[10],0x1800,1); //Clear Counter
		sleep(1);
		NewVMEWrite(VMEBusAccessMypoi[10],0x1804,1); //Save Counter
		
		if (MyLoop != 0) {
			system("clear");
		}

		//Master
		printf("\n");
		if (NewVMERead(VMEBusAccessMypoi[10], 0x2400) == 1) {
			printf("                  TAPS is in COUPLED MODE\n");
		} else {
			printf("                  TAPS is in STANDALONE MODE\n");
		}
		
		ActualPulserValue = NewVMERead(VMEBusAccessMypoi[10], 0x1000+8*4);
		if ((ActualPulserValue < 3000) || (ActualPulserValue > 3100)) {
			printf("\n\n\n\n        THIS PROGRAM IS RUNNING SOMEWHERE ELSE. CLOSE IT FIRST\n\n\nPulser value: %9lu (should be around 3070)\n", ActualPulserValue);
		} else {
			printf("\n");

			printf("  CFD  BaF2 [%0.2lx]   F %9lu   %9lu    E\n", NewVMERead(VMEBusAccessMypoi[10], 0x2500), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+5)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+4)*4));
			printf("                 A   %9lu   %9lu      D\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+0)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+3)*4));
			printf("                   B %9lu   %9lu    C\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+1)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+2)*4));
			
			printf("\n\n");
			
			printf("  LED1 BaF2 [%0.2lx]   F %9lu   %9lu    E\n", NewVMERead(VMEBusAccessMypoi[10], 0x2510), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+5+6*1)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+4+6*1)*4));
			printf("                 A   %9lu   %9lu      D\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+0+6*1)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+3+6*1)*4));
			printf("                   B %9lu   %9lu    C\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+1+6*1)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+2+6*1)*4));			

			printf("\n\n");
			
			printf("  LED2 BaF2 [%0.2lx]   F %9lu   %9lu    E\n", NewVMERead(VMEBusAccessMypoi[10], 0x2520), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+5+6*2)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+4+6*2)*4));
			printf("                 A   %9lu   %9lu      D\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+0+6*2)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+3+6*2)*4));
			printf("                   B %9lu   %9lu    C\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+1+6*2)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+2+6*2)*4));

			printf("\n\n");
			
			printf("  LED Veto  [%0.2lx]   F %9lu   %9lu    E\n", NewVMERead(VMEBusAccessMypoi[10], 0x2530), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+5+6*3)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+4+6*3)*4));
			printf("                 A   %9lu   %9lu      D\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+0+6*3)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+3+6*3)*4));
			printf("                   B %9lu   %9lu    C\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+1+6*3)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+2+6*3)*4));

			printf("\n\n");
			
			printf("  CFD PbWO4 [%0.2lx]   F %9lu   %9lu    E\n", NewVMERead(VMEBusAccessMypoi[10], 0x2540), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+5+6*4)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+4+6*4)*4));
			printf("                 A   %9lu   %9lu      D\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+0+6*4)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+3+6*4)*4));
			printf("                   B %9lu   %9lu    C\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+1+6*4)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+2+6*4)*4));
			
			printf("\n\n");
			
			printf("  CFD PWO-V [%0.2lx]   F %9lu   %9lu    E\n", NewVMERead(VMEBusAccessMypoi[10], 0x2550), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+5+6*5)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+4+6*5)*4));
			printf("                 A   %9lu   %9lu      D\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+0+6*5)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+3+6*5)*4));
			printf("                   B %9lu   %9lu    C\n", NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+1+6*5)*4), NewVMERead(VMEBusAccessMypoi[10], 0x1000+(32+2+6*5)*4));

			printf("\n");
			
			printf("          OR                          M2+\n");
			printf("CFD  BaF2 [%x]: %9lu(%9lu)   [%x]: %9lu(%9lu)\n",NewVMERead(VMEBusAccessMypoi[10], 0x2100),NewVMERead(VMEBusAccessMypoi[10], 0x1000+9*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+2*4), NewVMERead(VMEBusAccessMypoi[10], 0x2140),NewVMERead(VMEBusAccessMypoi[10], 0x1000+12*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+5*4));
			printf("LED1 BaF2 [%x]: %9lu(%9lu)   [%x]: %9lu(%9lu)\n",NewVMERead(VMEBusAccessMypoi[10], 0x2110),NewVMERead(VMEBusAccessMypoi[10], 0x1000+10*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+3*4),NewVMERead(VMEBusAccessMypoi[10], 0x2150),NewVMERead(VMEBusAccessMypoi[10], 0x1000+13*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+6*4));
			printf("LED2 BaF2 [%x]: %9lu(%9lu)   [%x]: %9lu(%9lu)\n",NewVMERead(VMEBusAccessMypoi[10], 0x2120),NewVMERead(VMEBusAccessMypoi[10], 0x1000+11*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+4*4),NewVMERead(VMEBusAccessMypoi[10], 0x2160),NewVMERead(VMEBusAccessMypoi[10], 0x1000+14*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+7*4));
			printf("LED Veto  [%x]: %9lu(%9lu)   [%x]: %9lu(%9lu)\n",NewVMERead(VMEBusAccessMypoi[10], 0x2130),NewVMERead(VMEBusAccessMypoi[10], 0x1000+23*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+21*4),NewVMERead(VMEBusAccessMypoi[10], 0x2170),NewVMERead(VMEBusAccessMypoi[10], 0x1000+24*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+22*4));
			printf("CFD PbWO4 [%x]: %9lu(%9lu)   [%x]: %9lu(%9lu)\n",NewVMERead(VMEBusAccessMypoi[10], 0x2190),NewVMERead(VMEBusAccessMypoi[10], 0x1000+29*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+25*4), NewVMERead(VMEBusAccessMypoi[10], 0x21a0),NewVMERead(VMEBusAccessMypoi[10], 0x1000+31*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+27*4));
			printf("CFD PWO-V [%x]: %9lu(%9lu)   [%x]: %9lu(%9lu)\n",NewVMERead(VMEBusAccessMypoi[10], 0x21b0),NewVMERead(VMEBusAccessMypoi[10], 0x1000+30*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+26*4),NewVMERead(VMEBusAccessMypoi[10], 0x21c0),NewVMERead(VMEBusAccessMypoi[10], 0x1000+20*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+28*4));
			printf("\n");
			printf("Pulser [%x]     :  %9lu   TAPS busy  :  %9lu   CB Coupled triggers:  %9lu\n",NewVMERead(VMEBusAccessMypoi[10], 0x2180),NewVMERead(VMEBusAccessMypoi[10], 0x1000+15*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+0*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+1*4));
			printf("TAPS L1 Trigger:  %9lu   TAPS L1 Int:  %9lu   Accepted to TAPS   :  %9lu\n",NewVMERead(VMEBusAccessMypoi[10], 0x1000+16*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+17*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+18*4));
			printf("\n");
			/*
			printf("Pulser [%x]  :    %9lu   TAPS L1 Trigger  : %9lu\n",NewVMERead(VMEBusAccessMypoi[10], 0x2180),NewVMERead(VMEBusAccessMypoi[10], 0x1000+15*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+16*4));
			printf("TAPS L1 Int :    %9lu   Accepted to TAPS : %9lu\n",NewVMERead(VMEBusAccessMypoi[10], 0x1000+17*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+18*4));
			printf("CB coupled triggers: %5lu   TAPS busy :        %9lu\n",NewVMERead(VMEBusAccessMypoi[10], 0x1000+1*4),NewVMERead(VMEBusAccessMypoi[10], 0x1000+0*4));
			*/
		};	
		
		if (argc > 1) {
			printf("\nStatus of signals: TAPSbusy = %i    CBInterrupt = %i\n\n",NewVMERead(VMEBusAccessMypoi[10], 0x2410)&0x1,(NewVMERead(VMEBusAccessMypoi[10], 0x2410)&0x2)>>1);
			
			printf("                             Status of individual channel masks:\n");

			printf(" Sec |      CFD BaF2     |     LED1 BaF2     |     LED2 BaF2     |     LED Veto      |PWO + PWO-V\n");
			printf("     |   64-33    32-1   |   64-33    32-1   |   64-33    32-1   |   64-33    32-1   |   32-1\n");
			printf("  A  | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx\n", NewVMERead(VMEBusAccessMypoi[1],0x2510), NewVMERead(VMEBusAccessMypoi[1],0x2500), NewVMERead(VMEBusAccessMypoi[1],0x2530), NewVMERead(VMEBusAccessMypoi[1],0x2520), NewVMERead(VMEBusAccessMypoi[1],0x2550), NewVMERead(VMEBusAccessMypoi[1],0x2540), NewVMERead(VMEBusAccessMypoi[7],0x2510), NewVMERead(VMEBusAccessMypoi[7],0x2500), NewVMERead(VMEBusAccessMypoi[7],0x2540));
			printf("  B  | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx |       \n", NewVMERead(VMEBusAccessMypoi[2],0x2510), NewVMERead(VMEBusAccessMypoi[2],0x2500), NewVMERead(VMEBusAccessMypoi[2],0x2530), NewVMERead(VMEBusAccessMypoi[2],0x2520), NewVMERead(VMEBusAccessMypoi[2],0x2550), NewVMERead(VMEBusAccessMypoi[2],0x2540), NewVMERead(VMEBusAccessMypoi[7],0x2530), NewVMERead(VMEBusAccessMypoi[7],0x2520));
			printf("  C  | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx\n", NewVMERead(VMEBusAccessMypoi[3],0x2510), NewVMERead(VMEBusAccessMypoi[3],0x2500), NewVMERead(VMEBusAccessMypoi[3],0x2530), NewVMERead(VMEBusAccessMypoi[3],0x2520), NewVMERead(VMEBusAccessMypoi[3],0x2550), NewVMERead(VMEBusAccessMypoi[3],0x2540), NewVMERead(VMEBusAccessMypoi[8],0x2510), NewVMERead(VMEBusAccessMypoi[8],0x2500), NewVMERead(VMEBusAccessMypoi[8],0x2540));
			printf("  D  | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx |       \n", NewVMERead(VMEBusAccessMypoi[4],0x2510), NewVMERead(VMEBusAccessMypoi[4],0x2500), NewVMERead(VMEBusAccessMypoi[4],0x2530), NewVMERead(VMEBusAccessMypoi[4],0x2520), NewVMERead(VMEBusAccessMypoi[4],0x2550), NewVMERead(VMEBusAccessMypoi[4],0x2540), NewVMERead(VMEBusAccessMypoi[8],0x2530), NewVMERead(VMEBusAccessMypoi[8],0x2520));
			printf("  E  | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx\n", NewVMERead(VMEBusAccessMypoi[5],0x2510), NewVMERead(VMEBusAccessMypoi[5],0x2500), NewVMERead(VMEBusAccessMypoi[5],0x2530), NewVMERead(VMEBusAccessMypoi[5],0x2520), NewVMERead(VMEBusAccessMypoi[5],0x2550), NewVMERead(VMEBusAccessMypoi[5],0x2540), NewVMERead(VMEBusAccessMypoi[9],0x2510), NewVMERead(VMEBusAccessMypoi[9],0x2500), NewVMERead(VMEBusAccessMypoi[9],0x2540));
			printf("  F  | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx | %0.8lx %0.8lx |       \n", NewVMERead(VMEBusAccessMypoi[6],0x2510), NewVMERead(VMEBusAccessMypoi[6],0x2500), NewVMERead(VMEBusAccessMypoi[6],0x2530), NewVMERead(VMEBusAccessMypoi[6],0x2520), NewVMERead(VMEBusAccessMypoi[6],0x2550), NewVMERead(VMEBusAccessMypoi[6],0x2540), NewVMERead(VMEBusAccessMypoi[9],0x2530), NewVMERead(VMEBusAccessMypoi[9],0x2520));
			printf("\n");
		}
		
		printf("\nHints: All values in Hz. Rates in ( ) are not prescaled. [] = prescaler setting\n");
		printf("       End with CTRL+C.\n");
		
	};


	//Close VMEbus in RAM for all 10 cards
	for (i=1;i<=10;i++) {
		CloseVMEBusForReadWrite(VMEBusAccessMypoi[i]);
	}

}
