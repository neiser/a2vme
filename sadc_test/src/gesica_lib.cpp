#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

#include "gesica_lib.h"


using namespace std;

bool i2c_wait(vme32_t gesica, bool check_ack) {
  // poll status register 0x4c bit 0,
  // wait until deasserted
  for(UInt_t n=0;n<100;n++) {
    UInt_t status = *(gesica+0x4c/4);
    if((status & 0x1) == 0) {
      //cout << "# After " << n << " reads: 0x4c = 0x" << hex << (status & 0x7f) << dec << endl;
      // check acknowledge bit
      if(check_ack && (status & 0x8) != 0) {
        cerr << "Address or data was not acknowledged " << endl;
        return false;
      }
      return true;
    }
  }
  cerr << "Did not see Status Bit deasserted, timeout." << endl;
  return false;
}

bool i2c_reset(vme32_t gesica) {
  // issue a reset, does also not help...
  cout << "# Resetting i2c state machine..." << endl;
  *(gesica+0x48/4) = 0x40;
  return i2c_wait(gesica, false);
}

void i2c_set_port(vme32_t gesica, UInt_t port_id, bool broadcast) {
  *(gesica+0x2c/4) = port_id & 0xff;
  if(broadcast) {
    *(gesica+0x50/4) = 0xff; // is this broadcast mode?
  }
  else {
    *(gesica+0x50/4) = port_id & 0xff;
  }
}

bool i2c_read(vme32_t gesica, UInt_t bytes, UInt_t addr, UInt_t& data) {
  // bytes must be between 1 and 4
  
  // write in address/control register (acr)
  // always set bit7 (trigger i2c transmission),
  // and bit4 (read mode)
  UInt_t acr = (1 << 7) + (1 << 4);
  // this nicely configures the i2c number of bytes
  acr |= bytes << 2;
  // set the address in the upper byte
  acr |= (addr << 8) & 0x7f00;
  //cout << "# Read Address/Control: 0x48 = 0x" << hex << acr << dec << endl;
  *(gesica+0x48/4) = acr;
  
  if(!i2c_wait(gesica, true))
    return false;
  
  
  // always read the lower data register 0x44
  data = *(gesica+0x44/4);
  
  // take care of upper data reg 0x58 and masking
  UInt_t mask = bytes % 2 == 0 ? 0xffff : 0xff;
  if(bytes>2) {
    data |= (*(gesica+0x58/4) & mask) << 16;
  }
  else {
    data &= mask;
  }
  return true;
}

bool i2c_write(vme32_t gesica, UInt_t bytes, UInt_t addr, UInt_t data) {
  // bytes must be between 1 and 4
  
  // always write 16bits to low register 0x40
  *(gesica+0x40/4) = data & 0xffff;
  
  // if needed, use upper data register 0x54
  if(bytes>2) {
    *(gesica+0x54/4) = (data >> 16) & 0xffff;
  }
  
  // write in address/control register (acr)
  // set bit7 (trigger i2c transmission),
  // but bit4=0 (write mode)
  UInt_t acr = 1 << 7;
  // assuming bytes>0 && bytes<4 (the caller should know that)
  // this nicely configures the i2c number of bytes
  acr |= bytes << 2;
  // set the address in the upper byte
  acr |= (addr << 8) & 0x7f00;
  //cout << "# Write Address/Control: 0x48 = 0x" << hex << acr << dec << endl;
  *(gesica+0x48/4) = acr;
  
  return i2c_wait(gesica, true);
}

UInt_t i2c_make_reg_addr(UInt_t adc_side, UInt_t reg) {
  // in order to access the registers of the
  // ZR chips via the HL chip on the SADC,
  // we set the highest bit6 in i2c addr
  UInt_t addr = 1 << 6;
  // select the chip via bit5 (adc_side=1 and adc_side=0)
  addr |= (adc_side & 0x1) << 5;
  // and set the actual register (bit4-bit0)
  addr |= reg & 0x1f;
  return addr;
}

bool i2c_read_reg(vme32_t gesica, UInt_t adc_side, UInt_t reg, UInt_t& data) {
  
  UInt_t addr = i2c_make_reg_addr(adc_side, reg);
  
  // this write before reading is really necessary...
  // does it tell the HL chip to access this address of one of the ZRs?
  if(!i2c_write(gesica, 1, addr, 0))
    return false;
  
  // read the response, maximum two bytes
  if(!i2c_read(gesica, 2, addr, data))
    return false;
  
  return true;
}

bool i2c_write_reg(vme32_t gesica, UInt_t adc_side, UInt_t reg, UInt_t data) {
  
  UInt_t addr = i2c_make_reg_addr(adc_side, reg);
  
  // tell the HL chip to access this address of the ZR?
  // this was not in the old code, but a complete i2c_read_reg read was before that
  // to make it more reliable........?!?!?!
  //if(!i2c_write(gesica, 1, addr, 0))
  //  return false;
  
  // construct our data, the lower 8 bits must be zero?
  UInt_t i2c_data = data << 8;
  if(!i2c_write(gesica, 3, addr, i2c_data))
    return false;
  
  return true;
}

bool load_rbt(const char* rbt_filename, vector<UInt_t>& data) {
  ifstream rbt_file(rbt_filename);
  if(!rbt_file.is_open()) {
    cerr << "Could not open RBT file " << rbt_filename << endl;
    return false;
  }
  cout << "Opened RBT file " << rbt_filename << endl;
  string line;
  UInt_t lineno = 0;
  UInt_t numOfBits = 0;
  while(getline(rbt_file,line)) {
    lineno++;
    if(lineno==7) {
      stringstream ss(line);
      ss >> line; //  dump leading string
      ss >> numOfBits;
    }
    if(lineno<=7)
      continue;
    
    // convert bit line to number
    bitset<32> bits(line);
    UInt_t word = bits.to_ulong();
    data.push_back((word >> 24) & 0xff);
    data.push_back((word >> 16) & 0xff);
    data.push_back((word >>  8) & 0xff);
    data.push_back((word >>  0) & 0xff);
  }
  // check
  if(data.size() != numOfBits/8) {
    cerr << "Not enough bits read as promised in header" << endl;
    return false;
  }
  return true;
}


bool init_gesica(vme32_t gesica, 
                 std::vector<UInt_t>& ports, 
                 bool skip_i2c) {
  
  // the CPLD module ID is at 0x0, should be 0x440d5918
  if(*(gesica+0x0/4) != 0x440d5918) {
    cerr << "Gesica firmware invalid, wrong address?" << endl;
    return false;
  }
  
  // if the Gesica FPGA itself is programmed, 
  // the TCS (and thus i2c) might not be working...
  if(skip_i2c)
    return true;    
  
  // check if clocks are locked (if not there's a ConfigTCS needed...)
  // SCR = status and control register
  UInt_t gesica_SCR = *(gesica+0x20/4);
  if((gesica_SCR & 0x1) == 0) {
    cerr << "TCS clock not locked: Status = 0x"
         << hex << gesica_SCR << dec << endl;
    return false;
  }
  if((gesica_SCR & 0x2) == 0) {
    cerr << "Internal clocks not locked. Status = 0x"
         << hex << gesica_SCR << dec << endl;
    return false;
  }
  
  // reset the i2c...
  if(!i2c_reset(gesica)) {
    cerr << "I2C reset failed" << endl;
    return false;
  }
  
  // Init connected iSADC cards
  for(UInt_t port_id=0;port_id<8;port_id++) {
    if( (gesica_SCR & (1 << (port_id+8))) == 0) {
      // silently ignore unconnected cards...
      continue;
    }
    // set to broadcast mode
    i2c_set_port(gesica, port_id, true);
    // read hardwired id
    UInt_t hard_id;
    if(!i2c_read(gesica, 1, 0x0, hard_id))
      continue;
    // write geo id as port_id:
    // hard_id as the lower 8 bits, port id the higher 8 bits!
    if(!i2c_write(gesica, 2, 0x1, hard_id  + (port_id <<8)))
      continue;
    // readback geo id
    UInt_t geo_id;
    if(!i2c_read(gesica, 1, 0x1, geo_id))
      continue;
    if(geo_id != port_id) {
      cerr << "Setting Geo ID for port " << port_id << " failed: GeoID=" << geo_id << endl;
      continue;
    }
    ports.push_back(port_id);
    // enable interface for readout (two VME accesses, too lazy too use bitset here...)
    *(gesica+0x20/4) |= 1 << (port_id+16);
  }
  if(ports.empty()) {
    cerr << "No connected SADC modules found." << endl;
    return false;
  }
  return true;
}
