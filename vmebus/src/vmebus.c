#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "vmebus.h"


void* vmebus(off_t padd, size_t size)
{
  int vmebusFd = open("/dev/mem", O_RDWR);

  if (vmebusFd == -1) {
    return NULL;
  }

  void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    vmebusFd, padd);

  if (mem == MAP_FAILED) {
    return NULL;
  }

  return mem;
}

void* vmestd(off_t padd, size_t size)
{
  return vmebus(padd | VMEBUSPC_STDOFFSET, size);
}

void* vmeext(off_t padd, size_t size)
{
  return vmebus(padd | VMEBUSPC_EXTOFFSET, size);
}

void* vmesio(off_t padd, size_t size)
{
  return vmebus(padd | VMEBUSPC_SIOOFFSET, size);
}
