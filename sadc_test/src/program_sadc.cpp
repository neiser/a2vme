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

bool program_sadc(vme32_t gesica, const vector<UInt_t>& rbt_data) {
 
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
  
  // load the RBT file
  vector<UInt_t> rbt_data;
  if(!load_rbt(argv[2], rbt_data)) {
    cerr << "Cannot load RBT file...exit" << endl;
    exit (EXIT_FAILURE);      
  }
  
  // open VME access to GeSiCa at 
  vme32_t gesica = (vme32_t)vmestd(base_address, 0x1000);
  if (gesica == NULL) {
    cerr << "Error opening VME access to GeSiCa." << endl;
    exit (EXIT_FAILURE);
  }
  
  // find connected SADCs, and do some other checks...
  vector<UInt_t> ports;
  if(!init_gesica(gesica, ports)) {
    cerr << "Some problem with init'ing the Gesica, see output above...exit" << endl;
    exit (EXIT_FAILURE);
  }
  
  cout << "GeSiCa found, clocks locked. Start programming..." << endl;
  
  for(size_t i=0; i<ports.size();i++) {
    cout << ">>>> Programming SADC at Port = " << ports[i] << endl;
    i2c_set_port(gesica, ports[i]);
    if(!program_sadc(gesica, rbt_data)) {
      cerr << "Could not successfully program, exit." << endl;
      exit(EXIT_FAILURE);
    }
  }
  
}
