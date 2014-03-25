/*
  linux
*/

#ifndef _VMEMAPPER_H

#define _VMEMAPPER_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <vme_io.h>
#include <netinet/in.h>
#define MAX_VME_DEVS 4

extern void *MapVME(unsigned long, int, struct vme_mapping_ctrl);
extern void UnmapVME();

#ifdef CPP
extern void UnmapVME(unsigned long mapped_data, int size);
#endif

#endif	// _VMEMAPPER_H

