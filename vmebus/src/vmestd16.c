#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "vmebus.h"

void warte(volatile d);

int
main(argc, argv)
int argc;
char *argv[];
{
  int i;
  unsigned long addr, rest;
  unsigned long pattern;
  char * rw;
  int fd;
  volatile unsigned short *poi;

  if (argc != 4) {
    printf("Aufruf = vmestd16  Addr. (Hex) Pattern (Hex) r, w, rl oder wl\n");
    exit(1);
  }

  sscanf(argv[1], "%x\n", &addr);
  sscanf(argv[2], "%x\n", &pattern);
  rw = argv[3];
  // Adresse ist nur modulo 0x1000 moeglich
  // der Rest muss aufaddiert werden
  rest = addr % 0x1000;
  addr = (addr / 0x1000) * 0x1000;


  if ((poi = vmestd(addr, 0x1000)) == NULL) {
    perror("Fehler beim Device oeffnen");
    exit (-1);
  }

  poi += (rest / 2);
  if (!strncmp(rw, "w", 2)) {
    *poi = pattern;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
  }

  if (!strncmp(rw, "r", 2)) {

    pattern = *poi;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
  }

  if (!strncmp(rw, "wl", 2))  {

    *poi = pattern;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
    while(1)  {
      *poi = pattern;
      warte(100);
    }
  }

  if (!strncmp(rw, "rl", 2))  {

    pattern = *poi;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
    while(1)  {
      pattern = *poi;
      warte(100);
    }
  }

}

void warte(d)
volatile d;
{
  while (d) {
    d--;
  }
}
