#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "vmebus.h"

void warte(volatile d);

int main(int argc, char *argv[])
{
  int i;
  unsigned long addr, rest;
  unsigned long pattern;
  char * rw;
  int fd;
  volatile unsigned long *poi;

  if (argc != 4) {
    printf("Aufruf = vmeext  Addr. (Hex) Pattern (Hex) r, w, rl oder wl\n");
    exit(1);
  }

  sscanf(argv[1], "%x\n", &addr);
  sscanf(argv[2], "%x\n", &pattern);
  rw = argv[3];

  // obere 3 Bits setzen via Register 0xaa000000
  if ((poi = vmebus(0, 0xaa000000, 0x1000)) == NULL) {
    perror("Fehler beim Device oeffnen");
    exit (-1);
  }
  *poi = addr & 0xe0000000;
  // Adresse ist nur modulo 0x1000 moeglich
  // der Rest muss aufaddiert werden
  addr &= 0x1fffffff;	// obere Bits ausmaskieren
  rest = addr % 0x1000;
  addr = (addr / 0x1000) * 0x1000;

  if ((poi = vmeext(addr, 0x1000)) == NULL) {
    perror("Fehler beim Device oeffnen");
    exit (-1);
  }
  poi += (rest / 4);

  if (!strncmp(rw, "w", 2)) {
    *poi = pattern;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
  }

  if (!strncmp(rw, "r", 2))	{

    pattern = *poi;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
  }

  if (!strncmp(rw, "wl", 2))	{

    *poi = pattern;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
    while(1)	{
      *poi = pattern;
      warte(0);
    }
  }

  if (!strncmp(rw, "rl", 2))	{

    pattern = *poi;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
    while(1)	{
      pattern = *poi;
      warte(0);
    }
  }

}

void warte(volatile d)
{
  while (d) {
    d--;
  }
}


