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
void readout_gesica(vme32_t gesica, gesica_result_t& r, bool dump_spybuffer = false) {
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
  
  if(!dump_spybuffer)
    return;
  
  cout << "==== DUMPING SPYBUFFER (in hex) ====" << endl;
  for(size_t i=0; i<spybuffer.size();i++) {
    cout << hex 
         << setfill('0') << setw(8)  
         << i << " " 
         << setfill('0') << setw(8)
         << spybuffer[i] 
         << dec << endl;
  }  
  cout << "==== END SPYBUFFER ====" << endl;
}

int main(int argc, char *argv[])
{
  if (argc != 1) {
    cerr << "This program does not any arguments." << endl;
    exit(EXIT_FAILURE);
  }
  
  // open VME access to VITEC at base address 0x0, size 0x1000
  // Short I/O = 16bit addresses, 16bit data
  vme16_t vitec = (vme16_t)vmesio(0x0, 0x1000);
  if (vitec == NULL) {
    cerr << "Error opening VME access to VITEC." << endl;
    exit (EXIT_FAILURE);
  }
  // the firmware ID is at 0xe, should be 0xaa02
  if(*(vitec+0xe/2) != 0xaa02) {
    cerr << "VITEC firmware not 0xaa02...exit" << endl;
    exit(EXIT_FAILURE);
  }
  
  // open VME access to GeSiCa at 
  // base adress 0xdd1000 (vme-cb-adc-1a), size 0x1000
  vme32_t gesica = (vme32_t)vmestd(0xdd1000, 0x1000);
  if (gesica == NULL) {
    cerr << "Error opening VME access to GeSiCa." << endl;
    exit (EXIT_FAILURE);
  }
  
  vector<UInt_t> ports;
  if(!init_gesica(gesica, ports)) {
    cerr << "Detecting the SADC modules went wrong..." << endl;
    exit(EXIT_FAILURE);
  }
  
  // enable readout via VME, but disable everything else like debugging pulsers
  *(gesica+0x20/4) = 0x4;
  
  // only readout port 0 (init_gesica above enables all it can find)
  
  
  // set registers according to Igor
  for(UInt_t i=0;i<ports.size();i++) {
    UInt_t port_id = ports[i];
    i2c_set_port(gesica, port_id);    
    for(UInt_t adc_side=0;adc_side<2;adc_side++) {
      // set latency to 89=69+20, maybe we catch some NaI signals..
      i2c_write_reg(gesica, adc_side, 0x0, 89);        
      // 10 samples per event
      i2c_write_reg(gesica, adc_side, 0x1, 10);
      // set latch all mode 
      // 0x0 = for firmware 100
      // 0x1 = for firmware 104 ???
      i2c_write_reg(gesica, adc_side, 0x3, 0);
      
      // set baseline integral
      i2c_write_reg(gesica, adc_side, 0x4, 0);
      i2c_write_reg(gesica, adc_side, 0x5, 2);
      
      // set signal integral
      i2c_write_reg(gesica, adc_side, 0x6, 0);
      i2c_write_reg(gesica, adc_side, 0x7, 10);
      
      // set tail integral
      i2c_write_reg(gesica, adc_side, 0x8, 5);
      i2c_write_reg(gesica, adc_side, 0x9, 5);      
            
      // set all thresholds to zero
      for(UInt_t reg=0x10;reg<0x20;reg++) {
        i2c_write_reg(gesica, adc_side, reg, 0);        
      }
    } 
  }
  
  // read all registers
  for(UInt_t i=0;i<ports.size();i++) {
    UInt_t port_id = ports[i];
    i2c_set_port(gesica, port_id);
    for(UInt_t adc_side=0;adc_side<2;adc_side++) {
      // dump config/status regs
      for(UInt_t reg=0x0;reg<0xa;reg++) {
        UInt_t data;
        i2c_read_reg(gesica, adc_side, reg, data);
        
        cout << "Port=" << port_id
             << "ADC side=" << adc_side
             << hex << ": config 0x" << reg
             << "=0x" << data << dec << endl;
      }
      // dump thresholds
      for(UInt_t reg=0x10;reg<0x20;reg++) {
        UInt_t data;
        i2c_read_reg(gesica, adc_side, reg, data);
        
        cout << "Port=" << port_id
             << "ADC side=" << adc_side
             << hex << ": threshold 0x" << reg
             << "=0x" << data << dec << endl;
      }
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
    
    break;
    
  }
  
}

