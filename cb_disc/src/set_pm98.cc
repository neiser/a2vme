//g++ -Wall set_pm98.cc vme++.o lpm98.o -o set_pm98 -L/lib/ces -lvme

#include <iostream>
#include <stdio.h>

#include "vme++.h"
#include "lpm98.h"
#include "dsm.h"

using namespace std;

class Tpm98ui{
  // I know: It would have been more clever to inherit this
  // class as "ui is a discriminator". But I did not do this.
public:
  int Display(Tpm98 &disc);
  int GetChoice();
  int Process(Tpm98 &disc, int choice);
};

int Tpm98ui::Display(Tpm98 &disc){
  //  cout << "\033c"; // clear screen
  cout << "\n";
  cout << "\t+----------------------------------------+\n";
  cout << "\t|                                        |\n";
  cout << "\t|  Change settings of PM98-Discriminator |\n";
  cout << "\t|                                        |\n";
  cout << "\t|                    v0.3b 18-02 04 -dk- |\n";
  cout << "\t+----------------------------------------+\n\n";

  cout << "\t Branch: " << disc.GetBranchNo() 
       << "\t Device: " << disc.GetDeviceAddress() << "\n\n";
  cout << "  [01]\t set adc reference         REMEMBER TO DO THIS!\n";
  cout << "  [02]\t display thresholds\n";
  cout << "  [03]\t set thresholds\n";
  cout << "  [04]\t display mask\n";
  cout << "  [05]\t set mask\n";
  cout << "  [06]\t display pulse width          !experimental!"
    " (not tested!)\n";
  cout << "  [07]\t set pulse width              !experimental!\n";
  cout << "  \n";
  cout << "  [13]\t display pulse width and mask (expert)\n";
  cout << "  [14]\t set pulse width and mask     (expert)\n";
  cout << "  \n";
  cout << "  [55]\t threshold loop\n";
  cout << "  \n";
  cout << "  [60]\t set test-pulse amplitude     !experimental!"
    " (not tested!)\n";
  cout << "  [61]\t get test-pulse amplitude     !experimental!"
    " (not tested!)\n";
  cout << "  [62]\t perform electrical test      !experimental!"
    " (not tested!)\n";
  cout << "  [63]\t test the outputs             !experimental!\n";
  cout << "  \n";
  //  cout << "  [04]\t set mask\n";
  cout << "  [99]\t change active branch/device\n";
  cout << "  \n";
  cout << "  [00]\t quit\n";
  cout << endl;
  
  return 0;
}

int Tpm98ui::GetChoice(){
  int choice;
  cout << "  your choice: ";
  cin >> choice;
  return choice;
}

int Tpm98ui::Process(Tpm98 &disc, int choice){
  switch (choice){
  case 1: {                                  // set ADC reference
    unsigned short reference = 0x0000;
    cout << "Enter reference: 0x";

    //BAYA  cin >> hex >> reference >> dec;   //
    scanf("Enter Hex Reference");  //<<-----------  BAYA 
    disc.SetAdcReference(reference); 
    // not impemented:
    // cout << "SetAdcReference()=0x" << hex << reference << dec << "\t"; 
    // reference = 0x0000;
    // reference = disc.GetAdcReference();
    // cout << "GetAdcReference()=0x" << hex << reference << dec << endl;
  }
  break;

  case 2: {                                  // display thresholds
    unsigned short data_l, data_h;
    cout << "Read-back thresholds..." << endl;
    data_l = disc.GetLowThreshold();
    data_h = disc.GetHighThreshold();
    cout << "Low  Thr.: " << data_l << endl;
    cout << "High Thr.: " << data_h << endl;
    cout << endl;   
  }
  break;

  case 3: {                                 // set thresholds
    unsigned short low_thr  = 0;
    unsigned short high_thr = 0;
    unsigned short data_l, data_h;
    cout << "Set low-threshold to:  ";

    cin >> low_thr;  
    cout << "Set high-threshold to: ";
    cin >> high_thr;

    cout << "Setting thresholds..." << endl;
    disc.SetLowThreshold(low_thr);
    disc.SetHighThreshold(high_thr);
    cout << "Read-back thresholds..." << endl;
    data_l = disc.GetLowThreshold();
    data_h = disc.GetHighThreshold();
    cout << "Low  Thr.: Set: " << low_thr  << ", Read : " << data_l << endl;
    cout << "High Thr.: Set: " << high_thr << ", Read : " << data_h << endl;
    cout << endl;
  }
  break;   

  case 4: {                                // display mask
    unsigned short mask = 0x0000;
    mask = disc.GetMask();
    //BAYA cout << "GetMask()=0x" << hex << mask << dec << endl;  //<-- A Revoir BAYA
    cout << "GetMask()=0x"  << mask << endl;
  }
  break;

  case 5: {                               // set mask
    unsigned short mask = 0x0000;
    cout << "Enter mask: 0x";

    // BAYA cin >> hex >> mask >> dec;        //<-- A Revoir BAYA
    cin >> mask ;

    disc.SetMask(mask);
 
    // BAYA cout << "SetMask()=0x" << hex << mask << dec << "\t";        //<-- A Revoir BAYA
    cout << "SetMask()=0x" << mask << "\t";       


    mask = disc.GetMask();

    // BAYA cout << "GetMask()=0x" << hex << mask << dec << endl;        //<-- A Revoir BAYA
        cout << "GetMask()=0x" << mask << endl; 
  }
  break;

  case 6: {                               // display pw
    unsigned short pulse_width = 0x0000;
    pulse_width = disc.GetPulseWidth();

    //BAYA cout << "GetPulseWidth()=0x" << hex << pulse_width << dec << endl;       //<-- A Revoir BAYA
    cout << "GetPulseWidth()=0x" <<  pulse_width  << endl;

  }
  break;

  case 7: {                               // set pw
    unsigned short pulse_width = 0x0000; 
    cout << "Enter pulse width: 0x";

    // BAYA cin >> hex >> pulse_width >> dec;      //<-- A Revoir BAYA
    cin >> pulse_width ;

    disc.SetPulseWidth(pulse_width); 

    //BAYA cout << "SetPulseWidth()=0x" << hex << pulse_width << dec << "\t";     //<-- A Revoir BAYA
    cout << "SetPulseWidth()=0x" << pulse_width << "\t";

    pulse_width = 0x0000;
    pulse_width = disc.GetPulseWidth();

    // BAYA cout << "GetPulseWidth()=0x" << hex << pulse_width << dec << endl;     //<-- A Revoir BAYA
    cout << "GetPulseWidth()=0x" << pulse_width << endl;

  }
  break;

  case 13: {                               // get pw and mask
    unsigned long data = 0x00000000;
    data = disc.GetMaskNDuration();

    //BAYA cout << "GetMaskNDuration=0x" << hex << data << dec << endl;    //<-- A Revoir BAYA
    cout << "GetMaskNDuration=0x" << data << endl;
  }
  break;

  case 14: {                              // set pw and mask
    unsigned long data = 0x00000000;
    cout << "Enter four bytes for pw and mask: 0x";

    // BAYA cin >> hex >> data >> dec;           //<-- A Revoir BAYA
    cin >> data ;

    DEBUG_PRINT_S("Next: pw and mask");
    disc.SetMaskNDuration(data);
    data = 0x00000000;
    DEBUG_BREAK_S("Next: read back pw and mask");
    data = disc.GetMaskNDuration();

    // BAYA cout << "GetMaskNDuration=0x" << hex << data << dec << endl;    //<-- A Revoir BAYA
    cout << "GetMaskNDuration=0x" << data << endl;
  }
  break;

  case 60: {                             // set test pulse amplitude
    unsigned short amplitude = 0x00;
    cout << "Enter pulse amplitude (one byte): 0x";

    // BAYA cin >> hex >> amplitude >> dec;           //<-- A Revoir BAYA
    cin >> amplitude ;

    disc.SetTestPulseAmplitude(amplitude); 

    // BAYA cout << "SetTestPulseAmplitude()=0x" << hex << amplitude << dec << "\t";       //<-- A Revoir BAYA
    cout << "SetTestPulseAmplitude()=0x" << amplitude  << "\t";

    amplitude = 0x00;
    amplitude = disc.GetTestPulseAmplitude();

    // BAYA cout << "GetTestPulseAmplitude()=0x" << hex << amplitude << dec << endl;      //<-- A Revoir BAYA
    cout << "GetTestPulseAmplitude()=0x"  << amplitude << endl;
  }
  break;

  case 61: {                             // get test pulse amplitude
    unsigned short amplitude = 0x00;
    amplitude = disc.GetTestPulseAmplitude();

    // BAYA cout << "GetTestPulseAmplitude()=0x" << hex << amplitude << dec << endl;   //<-- A Revoir BAYA
    cout << "GetTestPulseAmplitude()=0x" << amplitude << endl;
  }
  break;

  case 62: {                             // perform electrical test
    unsigned short response;
    response = disc.ElectricalTest();

    // BAYA cout << "Response was: 0x" << hex << response << dec << "." << endl;    //<-- A Revoir BAYA
    cout << "Response was: 0x" << response  << "." << endl;
  }
  break;

  case 63: {                             // test the otuputs
    disc.SetAdcReference(0xff); 
    disc.SetTestPulseAmplitude(0xff);    // or only 4 bits?
    for(unsigned int i=1; i<=0xffff; i *= 2){
      cout << "i :" << i << "\n";
      disc.SetMask( 0xffff & (~i) );
      //      cout << "--";
      disc.ElectricalTest();
    }
  }
  break;
  case 55: {                                 // threshold loop
    unsigned short start, stop;
    unsigned short low_thr  = 0;
    unsigned short high_thr = 0;
    unsigned short data_l, data_h;    
    cout << "Set low-threshold to:  ";
    cin >> low_thr;
    cout << "Set high-threshold to: ";
    cin >> high_thr;
    cout << "Setting thresholds..." << endl;
    cout << "Start: ";
    cin >> start;
    cout << "Stop:  ";
    cin >> stop;
    for(unsigned short i = start; i<=stop; i++){
      cout << "current device: " << i << endl;
      disc.SetDeviceAddress(i);
      disc.SetLowThreshold(low_thr);
      disc.SetHighThreshold(high_thr);
      cout << "Read-back thresholds..." << endl;
      data_l = disc.GetLowThreshold();
      data_h = disc.GetHighThreshold();
      cout << "Low  Thr.: Set: " << low_thr  << ", Read : " << data_l << endl;
      cout << "High Thr.: Set: " << high_thr << ", Read : " << data_h << endl;
      cout << endl;
      cout << "--> RETURN <--";
      getchar();
    }
  }
  break;   

  case 99: {
    unsigned short branch, address;       // The new branch no./dev. address
    cout << "Enter new Branch: (dec):";
    cin >> branch;
    cout << "Enter new Device-Address (dec):";
    cin >> address;
    disc.SetBranchNo(branch);
    disc.SetDeviceAddress(address);
  }
  break;
  case 0:                                // quit
    return -1;
    break;
  default:
    cout << "Not yet implemented. Press 'RETURN' to continue.";
    getchar();
    return 0;
    break;
  }
  cout << "Press 'RETURN' to continue.";
  getchar();
  return 0;
}


// -- main --

int main(){

  Tpm98 disc(0xf800, 0, 0, 1);
  
  Tpm98ui ui;
  do{
    ui.Display(disc);
  } while( ( ui.Process(disc, ui.GetChoice()) ) >= 0);

  return 0;

}
