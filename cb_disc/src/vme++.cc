// gcc -o vme++ vme++.cc -Wall -llynx -lstdc++

// gcc -c  vme++.cc -Wall 

// gcc -O3 -I/usr/include/machine vme++.cc -L/lib/ces -lvme -lstdc++ -ovme++ 
//
//Update     B.Ousena 29 Sep. 2012 ....  Start update code to run on Linux ..

#include <sys/types.h> 
//  #include <ces/vmelib.h>  // <<-------  BAYA 
//#include <machine/vmelib.h> // << ------ BAYA

#include <stdlib.h>
#include <iostream>

#include "vme++.h"
#include "dsm.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

using namespace std;

/* BAYA
static struct pdparam_master pd_param = {   // Page descriptor parameters
  1,	        // No VME Iack
  0,	        // No VME Read Prefetch
  0,	        // No VME Write Posting
  1,	        // Autoswap
  0,    	// Page Descriptor Number
  {0, 0, 0}	// Reserved Flags
};

 BAYA */


// -------------------------------------


TVME::TVME(const unsigned long device_base, 
	   const unsigned long device_size,
	   const unsigned long address_modifier )
{
  // cout << "C: TVME::TVME()  called" << endl;  //  BAYA mved the init of parameters.. areleft in .h
 

  if ( (device_size | address_modifier) != 0 )
    {

      fMemFd = -1;

        if ( ( fMemFd = open("/dev/mem", O_RDWR)) == -1 ) {
         printf( "ERROR : Virtual memory descriptor open ");
         exit(0);
       }

      addr = device_base;
      rest = addr % 0x1000;
      addr = (addr / 0x1000) * 0x1000;
  
       if ((mem = mmap(NULL, device_size, PROT_READ | PROT_WRITE, MAP_SHARED, fMemFd, addr | EVMEbusA16 )) ==  MAP_FAILED) {
                  printf("ERROR : Virtual-Physical Memory Map");
                  exit(0);
                }

       poi=(unsigned short *)mem;
       poi += (rest / 2);
       log_base = (unsigned long )poi;
        
    }

  else /* device was mapped externally */
    {
      log_base  = device_base;       // hold dev. virtual  base
      phys_base = device_base;       // hold dev. physical base (not nice!)
    }


}

//-----------------------------------------------------------------------

unsigned long TVME::Read(const unsigned long offset, const unsigned short data_mode){
//  BAYA mved the init of parameters.. areleft in .h

  volatile unsigned long *p;
  unsigned long res;

  p = (unsigned long *)(log_base + offset);
  switch (data_mode){
  case VME_D16:
    DEBUG_PRINT_S("Attempting to read in D16 mode from 0x" << hex << (phys_base + offset) << dec);

    //(unsigned short)res = *(unsigned short*)p;
    res = *(unsigned short*)p;
       
    DEBUG_PRINT_S("Read 0x" << hex << res << " from 0x" << (phys_base + offset) << dec << " in D16-mode."); 
    return res;
    break;

  case VME_D32:
    DEBUG_PRINT_S("Attempting to read in D32 mode from 0x" << hex << (phys_base + offset) << dec);
    res = *p;
    return res;
    DEBUG_PRINT_E("Read 0x" << hex << res << " from 0x" << (phys_base + offset) << dec << " in D32-mode.");
    break;
  }
  return 0;
} 


// -----------------------------------------------------------------------------

unsigned short TVME::Write(const unsigned long offset, const unsigned long data, 
			   const unsigned short data_mode){
//  BAYA mved the init of parameters.. areleft in .h

 volatile unsigned long *p;
 p = (unsigned long *)(log_base + offset);
  switch (data_mode){
  case VME_D16:
    DEBUG_PRINT_S("Write 0x" << hex << data << " to 0x" <<(phys_base + offset) << dec << " in D16 mode.");
    *(short *)p = (short)data;
    return 0;
    break;
  case VME_D32:
    DEBUG_PRINT_S("Write 0x" << hex << data << " to 0x" << (phys_base + offset) << dec << " in D32 mode.");
    *p=data;
    return 0;
    break;
  }
  return -1;
}  



// ---------------------------------------------------------------------------

unsigned long TVME::GetVirtBase(){
  return log_base;
}


// ----------------------------------------------------------------------------

TVME::~TVME(){
  // cout << "D: TVME::~TVME() called" << endl;
  void *fVirtAddr;

  if ( (phys_base|size) != 0 )
    // return_controller(log_base, size); <<<- BAYA
    
  fVirtAddr = (void*)log_base;
    if(munmap(fVirtAddr, size) == -1)
      {
	//printf("ERROR: Virtual-Memory Unmapping");  <<<-------- Baya commented this one, easiest way to get rid :-)
      exit(0);
      }
  else
    /* nothing */;
}


// --------------


// --------------------------------------------------------------------------------------------------


int vme_pp_main(){
  long io_flags;   
  unsigned long dummy; 
  unsigned long addr; 
  
  cout << endl << "This is vme++" << endl << endl;

  TVME vme(0xee000000, 0x10000, VME_A32_NP);   // v792 
  
  TVME ca(0xe0110000,0x10000, VME_A32_NP);    // catch
  
  TVME cbd(0x800000, 80000, VME_A24_NP);   // camac branch driver
  
  TVME fiadc(0x701300, 0x100, VME_A24_NP); // fiadc

  io_flags = cout.setf(ios::hex, ios::basefield); 

  cout << "v792:" << endl;
  addr = 0x802a; 
  dummy = vme.Read(addr, VME_D16);
  cout << "D16 --> " << dummy << endl;
  dummy = vme.Read(addr, VME_D32);
  cout << "D32 --> " << dummy << endl;

  dummy = vme.Write(0x1060, 0x42, VME_D16);
  dummy = vme.Read(0x1060, VME_D16);
  cout << "1060 (42)--> " << dummy << endl;
  cout << "CATCH:" << endl;
  dummy = ca.Read(0x0000, VME_D32);        // Read Catch Identity
  cout << "ca --> " << dummy << endl; 
  dummy = ca.Write(0x2480,0x08,VME_D32);   // Write GeoID (works only after catchinit!)
  dummy = ca.Read(0x1130,VME_D32);         // Read back GeoID
  cout << "GeoID (8)--> " << dummy << endl;

  cout << "CBD:" << endl;                  
  dummy = cbd.Read(0xe802, VME_D16);       // Read CAMAC Status Register of CBD
  cout << "csr (c83d)--> " << dummy << endl;
  
  
  cout << "FIADC64:" << endl;
  dummy = fiadc.Read(0x0, VME_D16);
  cout << "csr (8000)--> " << dummy << endl;

  //  cout.flags(io_flags);  <<--  BAYA

  return 0;
}



