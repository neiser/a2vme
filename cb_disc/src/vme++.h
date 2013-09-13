//
// VME++, object oriented library to capsule vme access, started in 
//        september 2002 by Dirk Krambrich.
//
//        Okt. 04 2002: DK: Added Debug-feature, fixed minor bug in constructor.
//
// To Do: -improve castings in read/write to comply with ANSI C++
//        -introduce possibility of setting "standard mode" for read/write


#ifndef VME_Plus_Plus_h
#define VME_Plus_Plus_h

#define VME_SIZE       0x1000  // standard size fo vme-device :)

// Address Modifier (List is incomplete, not all of the options have 
// been tested. Add and test your stuff)

#define VME_A16_U      0x29  /* A16 user access                   */
#define VME_A16_NP     0x2d  /* A16 non-privileged data access    */
#define VME_A24_NP     0x39  /* A24 non-privileged data access    */
#define VME_A32_NP     0x09  /* A32 non-privileged data access    */

#define VME_A32_NP_BLT 0x0b  /* A32 non-privileged block transfer */

//BAYA 

enum { EVMEbusA24 = 0x9e000000,
       EVMEbusA32 = 0x80000000,
       EVMEbusA16 = 0x9f000000 };
       
//ENDBAYA

// Width of data word (important for read/write access. certain modules support D32/D16 for 
// certain registers)

#define VME_D16 16
#define VME_D32 32

class TVME{             // general implementation of vme device
 protected:
  
public:
  TVME(const unsigned long device_base,                        // physical base in vme-space
       const unsigned long device_size = VME_SIZE,             // size in vme-space
       const unsigned long address_modifier = VME_A16_U); // VME_A16_NP);     // guess what...
  // ! specify (size = 0 & address_modifier = 0 to indicate that device was mapped externaly !

  ~TVME();
  
  unsigned long Read(const unsigned long offset,               // read access to device_base + offset
		     const unsigned short data_mode=VME_D16);  // width of the register: 16 or 32 bit

  unsigned short Write(const unsigned long offset,             // write access to device_base + offset
		       const unsigned long data,               // data to write
		       const unsigned short data_mode=VME_D16);// width of the register: 16 or 32 bit
  //                                                              NB: the write command returns 0 or -1, 
  //                                                              it does not read back the value 
  //                                                              previousy written. Some registers would
  //                                                              not like this...

  unsigned long GetVirtBase();                                 // log_base should be secret. 
  //                                                              You should not need to ask for this!

 private:
  unsigned long phys_base;                                     // hold dev. physical base
  unsigned long log_base;                                      // base in userspace
  volatile unsigned short *poi;
  unsigned long size; 
  unsigned long addr, rest;                                         
  int fMemFd;
  void *mem;
 
int Map(const unsigned long device_base,                    // map device to userspace
	  const unsigned long size,
	  const unsigned long address_modifier);
 
  //






};

#endif
