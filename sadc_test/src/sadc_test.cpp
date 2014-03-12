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

bool i2c_wait(vme32_t gesica) {
  // poll status register 0x4c bit 0,
  // wait until deasserted
  for(UInt_t n=0;n<100;n++) {
    if(*(gesica+0x50/4) & 0x1 == 0) 
      return true;
  }
  cerr << "Did not see Status Bit deasserted, timeout." << endl;
  return false;
}

UInt_t i2c_read(vme32_t gesica, UInt_t nSADC, 
                UInt_t adc_side, UInt_t addr) {
  // enable all SADC cards?
  // 0x50=EI2CPortSelect: Front-End ID select
  *(gesica+0x50/4) = 0xff;
  // 0x48=EI2CAddrCtl: i2C Address/Control
  // read 1 byte, no reset, initiate i2c
  // address 0xe
  *(gesica+0x48/4) = 0xe94;
  if(!i2c_wait(gesica))
    return 0xffff;
  
  // set to specific SADC 
  // 0x50=EI2CPortSelect: Front-End ID select
  *(gesica+0x50/4) = nSADC;
  // 0x2c=EIHlPort: HotLink select
  *(gesica+0x2c/4) = nSADC;
  
  // write in address/control
  // 0x98 = 2 bytes read, no reset, initiate i2c
  *(gesica+0x48/4) = 
      0x4000 + 
      ((adc_side << 13) & 0x2000) +
      ((addr << 8) & 0x1f00) 
      + 0x98;
  if(!i2c_wait(gesica))
    return 0xffff;
  
  // read 16bits from low register 0x44
  return *(gesica+0x48/4) & 0xffff;
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
  
  // dump i2c registers of connected iSADC cards
  for(UInt_t iSADC=0;iSADC<6;iSADC++) {
    for(UInt_t adc_side=0;adc_side<2;adc_side++) {
      for(UInt_t reg=0;reg<0x1f;reg++)
      cout << "# iSADC " << iSADC << ", Side " << adc_side 
           << ", Reg 0x" << hex << reg 
           << " = 0x"
           << i2c_read(gesica, iSADC, adc_side, 0x2) 
           << dec << endl;      
    }
  }
  
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

