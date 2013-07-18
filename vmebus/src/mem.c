#include <stdio.h>
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
  volatile unsigned long *poi;
  if (argc != 4) {
    printf("Aufruf = mem  Addr. (Hex) Pattern (Hex) r, w\n");
    exit(1);
  }
  sscanf(argv[1], "%x\n", &addr);
  sscanf(argv[2], "%x\n", &pattern);
  rw = argv[3];
  // Adresse ist nur modulo 0x1000 moeglich
  // der Rest muss aufaddiert werden
  rest = addr % 0x1000;
  addr = (addr / 0x1000) * 0x1000;

  if ((poi = vmebus(0, addr, 0x1000)) == NULL) {
    perror("Fehler beim Device oeffnen");
    exit (-1);
  }

  poi += (rest / 4);
  if (!strncmp(rw, "w", 2)) {
    *poi = pattern;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
  }
  if (!strncmp(rw, "r", 2))       {
    pattern = *poi;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
  }

}
