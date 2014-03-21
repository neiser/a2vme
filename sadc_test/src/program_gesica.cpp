#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

#include "gesica_lib.h"

using namespace std;

bool i2c_program_gesica(vme32_t gesica, const char* rbt_filename) {
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
  if (argc != 2) {
    cerr << "Help: program_gesica <GeSiCa VME base address> <RBT file to program>" << endl;
    exit(EXIT_FAILURE);
  }
  
}
