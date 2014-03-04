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

// returns the 32bit words in the spybuffer, 
// if some could be read...
void readout_gesica(vme32_t gesica, vector<UInt_t>& spybuffer) {
  // this is motivated by the SpyRead() method in TVME_GeSiCA.h
  
  // read status register,
  // wait until lowest bit is high
  // indicates that data buffer is not empty
  // BUT IS IT COMPLETE?!
  // since every read takes 1us, 
  // this is a timeout of 200us
  int status_tries = 0;
  UInt_t status1 = 0;
  while(true) {
    // do the actual read
    status1 = *(gesica+0x24/4);
    // check lowest bit
    if(status1 & 0x1) 
      break;
    if(status_tries==200) {
      cerr << "Reached " << status_tries 
           << " tries waiting for data, RESET!" << endl;
      // reset module! can this harm?
      *(gesica+0x0/4) = 1;
      return;
    }
    status_tries++;            
  }
  
  // read first header from 0x28
  UInt_t header1 = *(gesica+0x28/4);
  // ...should be zero!
  if(header1 != 0x0) {
    cerr << "First word 0x" << hex << header1 << dec
         << " was not zero, RESET!" << endl;
    // reset module! can this harm?
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // read second header from 0x28
  UInt_t header2 = *(gesica+0x28/4);
  // check error flag
  if(header2 & 0x80000000) {
    cerr << "Catch Error bit is set: 0x" << hex << header2 << dec
         << ", RESET!" << endl;
    // reset module! can this harm?
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // extract number of words from header
  UInt_t nWordHeader = header2 & 0xffff;
  // read status reg 0x24 again (should be the same as in status1)
  UInt_t status2 = *(gesica+0x24/4);
  // the next mask 0xfff is actually wrong, 
  // but it should not harm, since we always 
  // expect less than 4096=0xfff words
  UInt_t nWordStatus = (status2 >> 16) & 0xfff;
  if(nWordHeader != nWordStatus) {
    // this is the famous "Error 4" appearing very often...
    cerr << "nWords in status " << nWordStatus 
         << " does not match "
         << "nWords in header " << nWordHeader << endl;
    // currently, this is simply ignored
    // so no reset and no return
    // *(gesica+0x0/4) = 1;
    // return;
  }
  if(nWordHeader > 0x1000) {
    cerr << "nWords in header too large " << nWordHeader 
         << ", RESET!" << endl;
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // read the FIFO, store it in spybuffer
  // note that this can be wrong if 
  // nWordHeader and nWordStatus don't match, see above...
  for( UInt_t n=0; n<nWordHeader; n++ ) { 
    UInt_t datum = *(gesica+0x28/4);
    spybuffer.push_back(datum);
  }
  
  // check the trailer 
  if(spybuffer[nWordHeader-1] != 0xcfed1200) {
    cerr << "Last word in spybuffer does not match 0xcfed1200 != 0x" 
         << hex << spybuffer[nWordHeader-1] << dec 
         << ", RESET!" << endl;
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // check the status reg again
  UInt_t status3 = *(gesica+0x24/4);
  if(status3 != 0x0) {
    cerr << "Status reg 0x" << hex << status3 << dec
         << " did not become zero, RESET!" << endl;
    *(gesica+0x0/4) = 1;
    return;
  }
  
  // always do a reset at the moment, 
  // this is due to "Error 4", see above...
  *(gesica+0x0/4) = 1;
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
  cout << "VITEC Firmware (should be 0xaa02): 0x"
       << hex << *(vitec+0xe/2) << dec << endl;
  
  // open VME access to GeSiCa at 
  // base adress 0xdd1000 (vme-cb-adc-1a), size 0x1000
  
  vme32_t gesica = (vme32_t)vmestd(0xdd1000, 0x1000);
  if (gesica == NULL) {
    cerr << "Error opening VME access to GeSiCa." << endl;
    exit (EXIT_FAILURE);
  }
  // the module ID is at 0x0
  cout << "GeSiCa Firmware (should be 0x440d5918): 0x"
       << hex << *(gesica+0x0/4) << dec << endl;
  
  // Set ACK of VITEC low by default
  *(vitec+0x6/2) = 0;
  
  cout << "Waiting for triggers..." << endl;
  
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
    vector<UInt_t> spybuffer;
    readout_gesica(gesica, spybuffer);
    // ******* END GESICA READOUT
    
    // Wait for Serial ID received, bit4 should become high
    int WaitCnt = 0;
    while ( (*(vitec+0xc/2) & 0x10) == 0) { WaitCnt++;  }
    
    UInt_t EventID = *(vitec+0xa/2) << 16;
    EventID += *(vitec+0x8/2);
    
    // output spybuffer using EventID?
    // check that ID is consecutive?
    
    cout << "=======================================" << endl;
    cout << "Received Event, ID=" << EventID << endl;
    cout << "Dumping spybuffer: " << endl;
    for(size_t n=0;n<spybuffer.size();n++) {
      cout << setfill('0') << setw(8) << hex 
           << n << " " << spybuffer[n] << endl;
    }
    cout << "=======================================" << endl;
    
    
    // Set ACK of VITEC low,
    // indicates that we've finished reading event
    *(vitec+0x6/2) = 0;
    
  }
  
}

