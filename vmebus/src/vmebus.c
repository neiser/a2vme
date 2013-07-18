#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "vmebus.h"

static int  vmebusFd;

void *
vmebus(int access, off_t padd, size_t size)
{
  void  *mem;

  if ((vmebusFd = open("/dev/mem", O_RDWR)) == -1) {
    return (NULL);
  }

  if ((mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                  vmebusFd, padd)) ==  MAP_FAILED) {
    return (NULL);
  }

  return (mem);
}

void *
vmestd(off_t padd, size_t size)
{
  return (vmebus(VMESTD, padd | VMEBUSPC_STDOFFSET, size));
}

void *
vmeext(off_t padd, size_t size)
{
  return (vmebus(VMEEXT, padd | VMEBUSPC_EXTOFFSET, size));
}

void *
vmesio(off_t padd, size_t size)
{
  return (vmebus(VMESIO, padd | VMEBUSPC_SIOOFFSET, size));
}
