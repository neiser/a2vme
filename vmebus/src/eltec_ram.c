#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "vmebus.h"

void warte(volatile d);
#define	DELAY	0		//ca. 80ns (Geode)

int
main(argc, argv)
	int argc;
	char *argv[];
{
	unsigned long i;
	unsigned int addr;
	volatile unsigned long pattern, testpattern;
	volatile unsigned long *poi, *poi1;

	if (argc != 3) {
		printf("Aufruf = eltec_ram Adr. std | ext\n");
		exit(1);
	}

	sscanf(argv[1], "%x\n", &addr);

	if (!strncmp(argv[2], "std", 2))	{
		if ((poi = vmestd(addr, 0x200000)) == NULL) {
			perror("Fehler beim Device oeffnen");
			exit (-1);
		}
		printf("ELTEC RAM512-2 im Standartmodus (32-Bit)\n");
	}

	if (!strncmp(argv[2], "ext", 2))	{
		if ((poi = vmeext(0x2a000000, 0x200000)) == NULL) {
		// 2a | 80 von vmeext gibt aaxxxxxx = Register ob. Bits
			perror("Fehler beim Device oeffnen");
			exit (-1);
		}
		// Setze obere Adress-Bits
		*poi = addr & 0xe0000000;
		if ((poi = vmeext(addr & 0x1fffffff, 0x200000)) == NULL) {
			perror("Fehler beim Device oeffnen");
			exit (-1);
		}
		printf("ELTEC RAM512-2 im Extended Modus (32-Bit)\n");
	}

	poi1 = poi;

	printf("Speichertest mit Inkrement\n");
	for (i = 0; i < 0x1fffff / 4; i++)	{
//		warte(DELAY);
		*poi = i;
//		warte(DELAY);
		pattern = *poi++;
		if (pattern != i)	{
			printf("Fehler Soll = %x gelesen = %x\n", i, pattern);
			exit(-1);
		}

	}
	printf("fertig\n");

	poi = poi1;
	testpattern = 0xffffffff;
	printf("Speichertest mit Pattern %lx\n", testpattern);
	for (i = 0; i < 0x1fffff / 4; i++)	{
		warte(DELAY);
		*poi = testpattern;
		warte(DELAY);
		pattern = *poi++;
		if (pattern != testpattern)	{
		printf("Fehler Soll = %x gelesen = %x\n", testpattern, pattern);
			exit(-1);
		}

	}
	printf("fertig\n");

	poi = poi1;
	testpattern = 0;
	printf("Speichertest mit Pattern %lx\n", testpattern);
	for (i = 0; i < 0x1fffff / 4; i++)	{
		warte(DELAY);
		*poi = testpattern;
		warte(DELAY);
		pattern = *poi++;
		if (pattern != testpattern)	{
		printf("Fehler Soll = %x gelesen = %x\n", testpattern, pattern);
			exit(-1);
		}

	}
	printf("fertig\n");

	poi = poi1;
	testpattern = 0xaaaaaaaa;
	printf("Speichertest mit Pattern %lx\n", testpattern);
	for (i = 0; i < 0x1fffff / 4; i++)	{
		warte(DELAY);
		*poi = testpattern;
		warte(DELAY);
		pattern = *poi++;
		if (pattern != testpattern)	{
		printf("Fehler Soll = %x gelesen = %x\n", testpattern, pattern);
			exit(-1);
		}

	}
	printf("fertig\n");

	poi = poi1;
	testpattern = 0x55555555;
	printf("Speichertest mit Pattern %lx\n", testpattern);
	for (i = 0; i < 0x1fffff / 4; i++)	{
		warte(DELAY);
		*poi = testpattern;
		warte(DELAY);
		pattern = *poi++;
		if (pattern != testpattern)	{
		printf("Fehler Soll = %x gelesen = %x\n", testpattern, pattern);
			exit(-1);
		}

	}
	printf("fertig\n");
}

void warte(d)
volatile d;
{
	while (d)
		d--;
}
