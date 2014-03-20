#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>

extern "C" {
#include "vmebus.h"
}

typedef unsigned long UInt_t;
typedef volatile UInt_t* vme32_t;   
typedef volatile unsigned short* vme16_t;

using namespace std;

struct gesica_result_t {
  UInt_t nWordStatus;
  UInt_t nWordHeader;
  UInt_t nStatusTries;
  UInt_t nWordTries;
  UInt_t nFifoReads;
  UInt_t ErrorCode;
  UInt_t EventIDTCS;
};

// returns the 32bit words in the spybuffer, 
// if some could be read...
void readout_gesica(vme32_t gesica, gesica_result_t& r) {
  // this is motivated by the SpyRead() method in TVME_GeSiCA.h
  
  // read status register,
  // wait until lowest bit is high
  // indicates that data buffer is not empty
  // BUT IS IT COMPLETE?!
  // since every read takes 1us, 
  // this is a timeout of 200us
  UInt_t status1 = 0;
  while(true) {
    r.nStatusTries++;
    // do the actual read
    status1 = *(gesica+0x24/4);
    // check lowest bit
    if(status1 & 0x1) 
      break;
    if(r.nStatusTries==200) {
      r.ErrorCode |= 1 << 5;
      // reset module! can this harm?
      *(gesica+0x0/4) = 1;
      return;
    }                
  }
    
  // read first header from 0x28
  UInt_t header1 = *(gesica+0x28/4);
  // ...should be zero!
  if(header1 != 0x0) {
    r.ErrorCode |= 1 << 1;
    // reset module! can this harm?
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // read second header from 0x28
  UInt_t header2 = *(gesica+0x28/4);
  // check error flag
  if(header2 & 0x80000000) {
    r.ErrorCode |= 1 << 0;
    // reset module! can this harm?
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // extract number of words from header
  r.nWordHeader = header2 & 0xffff;
  // read status reg 0x24 again (should be the same as in status1)
  UInt_t status2 = *(gesica+0x24/4);
  // the next mask 0xfff is actually wrong, 
  // but it should not harm, since we always 
  // expect less than 4096=0xfff words
  r.nWordStatus = (status2 >> 16) & 0xfff;
  if(r.nWordHeader != r.nWordStatus) {
    // this is the famous "Error 4" appearing very often,
    // but actually this is not an error but can usually 
    // be cured if the status reg is read again...
    r.ErrorCode |= 1 << 4;
    UInt_t nWordStatus_again;
    while(true) {
      r.nWordTries++;            
      UInt_t status_again = *(gesica+0x24/4);
      nWordStatus_again = (status_again >> 16) & 0xfff;
      
      // check words again
      if(nWordStatus_again == r.nWordHeader) 
        break;
      if(r.nWordTries==200) {
        r.ErrorCode |= 1 << 7;
        *(gesica+0x0/4) = 1;
        return;
      }     
    }
    // we dont set r.nWordStatus again
    // since we want to keep this for debugging
    // in the following, only r.nWordHeader is used!
    
  }
  
  if(r.nWordHeader > 0x1000) {
    r.ErrorCode |= 1 << 2;
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // read the FIFO, store it in spybuffer
  // do this until we see the trailer
  vector<UInt_t> spybuffer;
  while(true) {
    r.nFifoReads++;
    UInt_t datum = *(gesica+0x28/4);
    spybuffer.push_back(datum);
    // check for the trailer 
    if(datum == 0xcfed1200) {
      break;
    }
    else if(r.nFifoReads>0x1000) {
      // this should never happen,
      // and it's a serious error
      r.ErrorCode |= 1 << 8;
      *(gesica+0x0/4) = 1;
      return;
    }
  }
    
  if(r.nFifoReads != r.nWordHeader) {
    // Don't know if this can happen at all...
    r.ErrorCode |= 1 << 9;
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // remember the TCS event ID for debugging...
  r.EventIDTCS = spybuffer[0];
  
  // check the status reg again
  UInt_t status3 = *(gesica+0x24/4);
  if(status3 != 0x0) {
    // drain the spybuffer, 
    // don't know if this is really meaningful
    // at this point...
    UInt_t n = 0;
    do {
      UInt_t datum = *(gesica+0x28/4);
      spybuffer.push_back(datum);
      n++;
      if(n == 0x1000) {
        r.ErrorCode |= 10;
        *(gesica+0x0/4) = 1;
        return;
      }
    }
    while(*(gesica+0x24/4) != 0x0);
    r.ErrorCode |= 1 << 6;
    *(gesica+0x0/4) = 1;
    return;
  }
}

bool i2c_wait(vme32_t gesica, bool check_ack) {
  // poll status register 0x4c bit 0,
  // wait until deasserted
  for(UInt_t n=0;n<100;n++) {
    UInt_t status = *(gesica+0x4c/4);
    if((status & 0x1) == 0) {
      cout << "# After " << n << " reads: 0x4c = 0x" << hex << (status & 0x7f) << dec << endl;
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

bool i2c_read(vme32_t gesica, UInt_t addr, UInt_t& data) {

  // write in address/control register (acr)
  UInt_t acr =
      ((addr << 8) & 0x7f00)
      + 0x98; // 0x98 = 2 byte read, no reset, initiate i2c
  cout << "# Address/Control: 0x48 = 0x" << hex << acr << dec << endl;
  *(gesica+0x48/4) = acr;

  if(!i2c_wait(gesica, true))
    return false;

  // read 16bits from low register 0x44
  data = *(gesica+0x44/4) & 0xffff;
  return true;
}

bool i2c_write(vme32_t gesica, UInt_t addr, UInt_t data) {

  // write 16bits to low register 0x40
  *(gesica+0x40/4) = data & 0xffff;

  // write in address/control register (acr)
  UInt_t acr =
      ((addr << 8) & 0x7f00)
      + 0x88; // 0x88 = 2 byte write, no reset, initiate i2c
  cout << "# Address/Control: 0x48 = 0x" << hex << acr << dec << endl;
  *(gesica+0x48/4) = acr;

  return i2c_wait(gesica, true);
}

int main(int argc, char *argv[])
{
  if (argc != 1) {
    cerr << "This program does not take arguments." << endl;
    exit(EXIT_FAILURE);
  }
  
  // open VME access to VITEC at base address 0x0, size 0x1000
  // Short I/O = 16bit addresses, 16bit data
  vme16_t vitec = (vme16_t)vmesio(0x0, 0x1000);
  if (vitec == NULL) {
    cerr << "Error opening VME access to VITEC." << endl;
    exit (EXIT_FAILURE);
  }
  // the firmware ID is at 0xe
  cout << "# VITEC Firmware (should be 0xaa02): 0x"
       << hex << *(vitec+0xe/2) << dec << endl;
  
  // open VME access to GeSiCa at 
  // base adress 0xdd1000 (vme-cb-adc-1a), size 0x1000
  
  vme32_t gesica = (vme32_t)vmestd(0xdd1000, 0x1000);
  if (gesica == NULL) {
    cerr << "Error opening VME access to GeSiCa." << endl;
    exit (EXIT_FAILURE);
  }
  // the module ID is at 0x0
  cout << "# GeSiCa Firmware (should be 0x440d5918): 0x"
       << hex << *(gesica+0x0/4) << dec << endl;
  
  if(!i2c_reset(gesica)) {
    cerr << "I2C reset failed" << endl;
    exit(EXIT_FAILURE);
  }
  

  // SCR = status and control register
  // enable readout via VME, but disable everything else like debugging pulsers
  *(gesica+0x20/4) = 0x4;
  UInt_t gesica_SCR = *(gesica+0x20/4);
  // check if clocks are locked (if not there's a TCS problem)
  if((gesica_SCR & 0x1) == 0) {
    cerr << "TCS clock not locked" << endl;
    exit(EXIT_FAILURE);
  }
  if((gesica_SCR & 0x2) == 0) {
    cerr << "Internal clocks not locked" << endl;
    exit(EXIT_FAILURE);
  }

  // Init connected iSADC cards
  for(UInt_t port_id=0;port_id<6;port_id++) {
    if( (gesica_SCR & (1 << (port_id+8))) == 0) {
      cerr << "Port ID " << port_id << " not connected." << endl;
      continue;
    }
    // set to broadcast mode
    i2c_set_port(gesica, port_id, true);
    // read hardwired id
    UInt_t hard_id;
    if(!i2c_read(gesica, 0x0, hard_id))
      continue;
    hard_id &= 0xff; // (only lower 8 bits)
    // write geo id as port_id:
    // hard_id as the lower 8 bits, port id the higher 8 bits!
    if(!i2c_write(gesica, 0x1, hard_id  + (port_id <<8)))
      continue;
    // readback geo id
    UInt_t geo_id;
    if(!i2c_read(gesica, 0x1, geo_id))
      continue;
    geo_id &= 0xff; // (only lower 8 bits)
    if(geo_id != port_id) {
      cerr << "Setting Geo ID for port " << port_id << " failed: GeoID=" << geo_id << endl;
      continue;
    }
    cout << "Port ID=" << port_id << ", "
         << "Hardwired ID=0x" << hex << hard_id << dec << ", "
         << "Geo ID=0x" << hex << geo_id << dec
         << endl;
    // enable interface for readout
    *(gesica+0x20/4) |= 1 << (port_id+16);
  }

  // read some registers
  for(UInt_t port_id=0;port_id<6;port_id++) {
    // no broadcast mode => access single SADCs
    i2c_set_port(gesica, port_id, false);


    for(UInt_t reg=0x0;reg<0x3;reg++) {
      UInt_t addr = reg;
      UInt_t data;
      if(!i2c_read(gesica, addr, data))
        continue;
      cout << "Port=" << port_id
           << hex << " 0x" << addr
           << "=0x" << data << dec << endl;
    }
  }

  exit(EXIT_SUCCESS);

  // Set ACK of VITEC low by default
  *(vitec+0x6/2) = 0;
  
  cout << "# Waiting for triggers..." << endl;
  cout << "# EventID EventIDTCS nWordStatus nWordHeader nStatusTries nWordTries nTrailerPos ErrorCode" << endl;
  
  while(true) {
    // Wait for INT bit of VITEC to become high
    // this indicates a trigger
    int TriggerSeen = (*(vitec+0xc/2) & 0x8000);
    if(!TriggerSeen)
      continue;
    
    // Set ACK of VITEC high
    // Indicate that we've seen the trigger,
    // and have started the readout
    *(vitec+0x6/2) = 1;
    
    
    // ******* START GESICA READOUT
    gesica_result_t r = {};
    readout_gesica(gesica, r);
    // ******* END GESICA READOUT
    
    // Wait for Serial ID received, bit4 should become high
    int WaitCnt = 0;
    while ( (*(vitec+0xc/2) & 0x10) == 0) { WaitCnt++;  }
    
    UInt_t EventID = *(vitec+0xa/2) << 16;
    EventID += *(vitec+0x8/2);
    
    cout << EventID << " "
         << r.EventIDTCS << " "
         << r.nWordStatus << " "
         << r.nWordHeader << " "
         << r.nStatusTries << " "
         << r.nWordTries << " "
         << r.nFifoReads << " "
         << r.ErrorCode << endl;
    
    // Set ACK of VITEC low,
    // indicates that we've finished reading event
    *(vitec+0x6/2) = 0;
    
  }
  
}

