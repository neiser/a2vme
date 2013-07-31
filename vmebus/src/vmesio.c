#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "vmebus.h"
#include <unistd.h>

int main(int argc, char *argv[])
{

  if (argc != 4) {
    printf("Usage: vmesio address pattern [r|w|rl|wl], all in hex\n");
    exit(1);
  }

  unsigned long addr, pattern;
  sscanf(argv[1], "%x\n", &addr);
  sscanf(argv[2], "%x\n", &pattern);
  char* rw = argv[3];
  // address for mmap call is aligned to PAGE_SIZE=0x1000
  // the remainder must be added manually
  unsigned long rest = addr % 0x1000;
  addr = (addr / 0x1000) * 0x1000;

  // volatile here ensure correct behaviour in loops
  volatile unsigned short *poi = vmesio(addr, 0x1000);
  if (poi == NULL) {
    perror("Error opening device. Are you root? Msg");
    exit (-1);
  }

  // let's see if that works in short IO mode
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
      usleep(1000);
    }
  }

  if (!strncmp(rw, "rl", 2))  {

    pattern = *poi;
    printf("ADDR = %0.8lx Pattern = %0.8lx RW = %s\n",
           addr, pattern, rw);
    while(1)  {
      pattern = *poi;
      usleep(1000);
    }
  }

}
