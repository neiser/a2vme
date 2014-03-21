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

bool program_sadc(vme32_t gesica, const char* rbt_filename)
{

  vector<UInt_t> rbt_data;
  if(!load_rbt(rbt_filename, rbt_data))
    return false;

  // reset FPGA
  if(!i2c_write(gesica, 1, 2, 0x0))
    return false;

  // set program bit = bit2
  if(!i2c_write(gesica, 1, 2, 0x4))
    return false;

  // wait for init
  UInt_t nTries = 0;
  UInt_t status = 0;
  do {
    if(!i2c_read(gesica, 1, 2, status))
      return false;
    nTries++;
    if(nTries==10000) {
      cerr << "Reached maximum wait time for init programming. "
           << "Last status = " << hex << status << dec << endl;
      return false;
    }
  }
  while((status & 0x1) == 0);

  // write the file, 2 bytes at once
  cout << "Programming SADC..." << endl;
  for(UInt_t i=0;i<rbt_data.size();i+=2) {
    UInt_t i2c_data = (rbt_data[i+1] << 8) + rbt_data[i];
    if(!i2c_write(gesica, 2, 3, i2c_data)) {
      cerr << "Failed writing at bytes=" << i << endl;
      return false;
    }
    if(i % (1 << 14) == 0)
      cout << "." << flush;
  }
  cout << endl;


  // check status, bit1 = FPGA0 done, bit3 = FPGA1 done
  if(!i2c_read(gesica, 1, 2, status))
    return false;
  if(status != 0xe) {
    cerr << "Status = " << hex << status  << dec
         << " != 0xe (not both FPGAs indicate DONE), programming failed." << endl;
    return false;
  }

  return true;
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    cerr << "Help: program_sadc <GeSiCa VME base address> <RBT file to program>" << endl;
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
  
  // open VME access to GeSiCa at 
  vme32_t gesica = (vme32_t)vmestd(base_address, 0x1000);
  if (gesica == NULL) {
    cerr << "Error opening VME access to GeSiCa." << endl;
    exit (EXIT_FAILURE);
  }
  
  // check firmware
  if(*(gesica+0x0/4) != 0x440d5918) {
    cerr << "Gesica firmware invalid, wrong address?" << endl;
    exit(EXIT_FAILURE);
  }
  
  // check if clocks are locked (if not there's a TCS problem)
  UInt_t gesica_SCR = *(gesica+0x20/4);
  if((gesica_SCR & 0x1) == 0) {
    cerr << "TCS clock not locked: Status = 0x"
         << hex << gesica_SCR << dec << endl;
    exit(EXIT_FAILURE);
  }
  if((gesica_SCR & 0x2) == 0) {
    cerr << "Internal clocks not locked. Status = 0x"
         << hex << gesica_SCR << dec << endl;
    exit(EXIT_FAILURE);
  }
  
  
}
