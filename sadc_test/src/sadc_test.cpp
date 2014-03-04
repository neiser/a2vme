#include <iostream>
#include <cstdlib>

extern "C" {
#include "vmebus.h"
}

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 1) {
        cerr << "This program does not take arguments." << endl;
        exit(EXIT_FAILURE);
    }

    // open VME access to VITEC at base address 0x0, size 0x1000
    // Short I/O = 16bit addresses, 16bit data
    typedef volatile unsigned short* vme16_t;
    vme16_t vitec = (vme16_t)vmesio(0x0, 0x1000);
    if (vitec == NULL) {
        cerr << "Error opening VME access to VITEC." << endl;
        exit (EXIT_FAILURE);
    }
    // the firmware ID is at 0xe
    cout << "VITEC Firmware (should be 0xaa02): 0x"
         << hex << *(vitec+0xe/2) << endl;

    // open VME access to GeSiCa at 
    // base adress 0xdd1000 (vme-cb-adc-1a), size 0x1000
    typedef volatile unsigned long* vme32_t;    
    vme32_t gesica = (vme32_t)vmestd(0xdd1000, 0x1000);
    if (gesica == NULL) {
        cerr << "Error opening VME access to GeSiCa." << endl;
        exit (EXIT_FAILURE);
    }
    // the module ID is at 0x0
    cout << "GeSiCa Firmware (should be 0x440d5918): 0x"
         << hex << *(gesica+0x0/4) << endl;
    
    // Set ACK of VITEC low by default
    *(vitec+0x6/2) = 0;

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

        
        // START GESICA READOUT
        
        
        
        // END GESICA READOUT
        
        // Wait for Serial ID received, bit4 should become high
        int WaitCnt = 0;
        while ( (*(vitec+0xc/2) & 0x10) == 0) { WaitCnt++;  }

        int EventID = (*(vitec+0xa/2) << 16);
        EventID += (*(vitec+0x8/2));

        // output event using EventID?
        // check that ID is consecutive?

        // Set ACK of VITEC low,
        // indicates that we've finished reading event
        *(vitec+0x6/2) = 0;

    }

}

