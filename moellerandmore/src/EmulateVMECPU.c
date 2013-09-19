#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "vmebus.h"
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

#define Verbose 1 //Set to 1 for detailed read/write information, if not to 0

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


	//open VITEC
	volatile unsigned short *poi = vmesio(0x0, 0x1000);
	if (poi == NULL) {
		perror("Error opening device. Are you root? Msg");
		exit (-1);
	}

	printf("VITEC Firmware: %x\n", *(poi+0xe/2));


	int Counter = 0;
	int Counter2 = 0;
	int CounterRec = 0;
	int CounterEventBefore = -1;
	int NumberOfErrors = 0;
	clock_t before = clock();
	clock_t result;
	*(poi+0x6/2) = 0; //Set ACK low
	while (-1) {
		Counter2++;

		int EventInt = 0;
		EventInt = (*(poi+0xc/2) & 0x8000); //Wait For INT bit to become high
//		EventInt = (*(poi+0xc/2) & 0x10);   //Wait for Serial ID received, bit4 should become high


		if ( EventInt ) { //Wait for Int
			Counter++;
			*(poi+0x6/2) = 1; //Set VITEC ACK high
			int WaitCnt = 0;
			while ( (*(poi+0xc/2) & 0x10) == 0) { WaitCnt++;  } //Wait for Serial ID received, bit4 should become high
			//if (WaitCnt > 10) printf("%d ", WaitCnt);
			
			unsigned short LastReg = *(poi+0xc/2);
			if ( (LastReg & 0xFF) != 0x1a) {
				printf("Wrong Status Register: %x\n", LastReg);
			}

			CounterRec = (*(poi+0xa/2) << 16);
			CounterRec += (*(poi+0x8/2));

			if ( (CounterRec-CounterEventBefore) != 1) {
				printf("Error in EventID receiving. Current event: %8x  event before: %8x\n", CounterRec, CounterEventBefore);
				NumberOfErrors++;
			}
			CounterEventBefore = CounterRec;
			
			//printf("%8d: %0.4x %0.8x\n", Counter, *(poi+0xc/2), CounterRec);
			*(poi+0x6/2) = 0; //Set ACK low
			if ((Counter % 1000) == 0) {
				result = clock() - before;
				printf("%d\t%i\t%i\t",NumberOfErrors, Counter,Counter2);
				printf("%f\n",1000/(((float)result)/CLOCKS_PER_SEC));
				before = clock();
			}
		}
	}
	
}

