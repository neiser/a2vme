#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

#include "gesica_lib.h"

extern "C" {
#include "vmebus.h"
}

using namespace std;

bool program_gesica(vme32_t gesica, const char* rbt_filename) {
  vector<UInt_t> rbt_data;
  if(!load_rbt(rbt_filename, rbt_data))
    return false;

  // the CPLD register 0x14 is used to program the GeSiCa FPGA
  // there are three relevant bits (see manual)

  // init chip
  *(gesica+0x14/4) = 0x7;
  *(gesica+0x14/4) = 0x2;

  // wait for init high (bit0)
  UInt_t nTries = 0;
  while((*(gesica+0x14/4) & 0x1) == 0) {
    nTries++;
    if(nTries==1000) {
      cerr << "Reached maximum wait time for init programming. "
           << "Last value 0x14 = " << hex << *(gesica+0x14/4) << dec << endl;
      return false;
    }
  }
  UInt_t status = *(gesica+0x14/4);
  cout << "Start programming, status = 0x" << hex << status << dec << endl;


  // transmit data bitwise, per byte msb first
  for(UInt_t i=0;i<rbt_data.size();i++) {
    for(int j=7;j>=0;j--) { // j must be signed here
      UInt_t bit = (rbt_data[i] >> j) & 0x1;
      // first write the bit, set CCLK low
      UInt_t datum = (bit << 2);
      *(gesica+0x14/4) = datum;
      // then set CCLK => rising edge of CCLK
      datum |= 0x2;
      *(gesica+0x14/4) = datum;
    }
    if(i % (1 << 14) == 0)
      cout << "." << flush;
  }
  cout << endl;

  // check status again
  status = *(gesica+0x14/4);
  if((status & 0x2) == 0) {
    cerr << "Status bit1 not high (not DONE), programming failed." << endl;
    return false;
  }
  cout << "Programming done, status = 0x" << hex << status << dec << endl;

  // try resetting the module..?!
  *(gesica+0x0/4) = 1;

  // wait for TCS clock to be locked
  nTries = 0;
  while((*(gesica+0x20/4) & 0x1) == 0) {
    nTries++;
    if(nTries==10000000) {
      cerr << "Reached maximum wait time for TCS clock lock. "
           << "Last value 0x20 = " << hex << *(gesica+0x20/4) << dec << endl;
      return false;
    }
  }


  return true;
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    cerr << "Help: program_gesica <GeSiCa VME base address> <RBT file to program>" << endl;
    exit(EXIT_FAILURE);
  }
  
  // parse base address of GeSiCa in hex
  stringstream ss(argv[1]);
  UInt_t base_address;
  if(!(ss >> hex >> base_address)) {
    cerr << "Cannot parse base address of GeSiCa" << endl;
    exit (EXIT_FAILURE);      
  }
  base_address <<= 12;
  
  // open VME access to GeSiCa at base_address
  vme32_t gesica = (vme32_t)vmestd(base_address, 0x1000);
  if (gesica == NULL) {
    cerr << "Error opening VME access to GeSiCa." << endl;
    exit (EXIT_FAILURE);
  }
  
  // ports are actually unused and skip i2c checking
  vector<UInt_t> ports;
  if(!init_gesica(gesica, ports, true)) {
    cerr << "Some problem with init'ing the Gesica, see output above...exit" << endl;
    exit (EXIT_FAILURE);
  }
  
  if(!program_gesica(gesica, argv[2])) {
    cerr << "Programming failed, see above." << endl;
    exit(EXIT_FAILURE);
  }
  
}
