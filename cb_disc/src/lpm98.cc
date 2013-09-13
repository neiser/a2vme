// Side effect: changing branch/device address programms the adc-reference!
// Dirk Krambrich, 22nd Apr. 2004: Swapped "low" and "high" threshold.
//
//
//
// Update     B.Ousena 29 Sep. 2012 ....  Start update code to run on Linux ..

//<<--- BAYA

#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include <sys/types.h>
#include <stdio.h>

#include "lpm98.h"
#include "dsm.h"

using namespace std;

#define I2C_DLY 80     /* no of µs before expecting        //<<<, ------------ BAYA had to find out the right delay ..
			  the i2c command is executed */

#define ADC_REFERENCE 0xff /* Reference for ADC */

#ifndef VERBOSE
#define VERBOSE 0      /* tell the truth about sending crap */
#endif

// -- PM98 Discriminator --

Tpm98::Tpm98(unsigned long controller_base,
	     unsigned short device_branch, unsigned short device_address,
	     int map_controller){
  DEBUG_PRINT_E("Tpm98::Tpm98");
  fControllerBase = controller_base;
  fBranch = device_branch;
  fAddress = 0xff & (device_address << 2);  // two least significant bits
  //                                           are reserved for interenal
  //                                           destination.
  if (map_controller)
    i2cComm = new Ti2cComm(fControllerBase);
  else
    i2cComm = new Ti2cComm(fControllerBase, 1);
  SetAdcReference(ADC_REFERENCE);          // do not forget this after coosing
  //                                          an other branch / device !!!
}

int Tpm98::SetParameters(unsigned short branch_no, unsigned short device_address,
			 unsigned short adc_reference = 0xff, 
			 unsigned short low_threshold = 50,  
			 unsigned short high_threshold = 50,
			 unsigned short pulse_width   = 0x0, 
			 unsigned short mask = 0x0, 
			 unsigned short output_mode = 0){
  SetBranchNo(branch_no);
  SetDeviceAddress(device_address);
  SetAdcReference(adc_reference);
  SetLowThreshold(low_threshold);
  SetPulseWidth(pulse_width);
  SetMask(mask);
  // SetOutputMode(output_mode & 0x1); // not impelmented now.
  return 0;
}



int Tpm98::SetAdcReference(unsigned short adc_reference = 0xff){
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);     // base_address_1 + 0
  i2cComm->SendByte(fAddress, fBranch); // base_address_2 + 0
  i2cComm->SendByte(0x0f, fBranch);     // choose to set ADC reference
  i2cComm->SendByte(adc_reference, fBranch);
  i2cComm->CloseBranch(fBranch);
  return 0;
}

int Tpm98::SetHighThreshold(unsigned short high_threshold){
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);     // base_address_1 + 0
  // cout << "fAddress2: 0x" << hex << fAddress << dec << endl;
  i2cComm->SendByte(fAddress, fBranch); // base_address_2 + 0
  i2cComm->SendByte(0x03, fBranch);     // choose to set high threshold
  i2cComm->SendByte(high_threshold, fBranch);
  i2cComm->CloseBranch(fBranch);
  return 0;
}

int Tpm98::SetLowThreshold(unsigned short low_threshold){
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);     // base_address_1 + 0
  i2cComm->SendByte(fAddress, fBranch); // base_address_2 + 0
  i2cComm->SendByte(0x07, fBranch);     // choose to set low threshold
  i2cComm->SendByte(low_threshold, fBranch);
  i2cComm->CloseBranch(fBranch);
  return 0;
}

int Tpm98::SetTestPulseAmplitude(unsigned short amplitude){
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);     // base_address_1 + 0
  i2cComm->SendByte(fAddress, fBranch); // base_address_2 + 0
  i2cComm->SendByte(0x0b, fBranch);     // choose to set tst pulse ampl.
  i2cComm->SendByte(amplitude, fBranch);
  i2cComm->CloseBranch(fBranch);
  return 0;
}

unsigned short Tpm98::GetHighThreshold(){
  unsigned short i2cStatus = 0;
  unsigned short data;
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);            // base_address_1 + 0
  i2cComm->SendByte(fAddress + 0x01, fBranch); // base_address_2 + 1
  i2cComm->SendByte(0x8f, fBranch);            // choose to read high threshold
  i2cComm->CloseBranch(fBranch);

  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf1, fBranch);            // base_address_1 + 1
  i2cComm->SendByte(fAddress + 0x01, fBranch); // base_address_2 + 1
  i2cComm->RecvByte(fBranch, i2cStatus);       // recv 0x00;
  data = i2cComm->RecvLastByte(fBranch, i2cStatus);
  i2cComm->CloseBranch(fBranch);

  return data;
}

unsigned short Tpm98::GetLowThreshold(){
  unsigned short i2cStatus = 0;
  unsigned short data;
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);            // base_address_1 + 0
  i2cComm->SendByte(fAddress + 0x01, fBranch); // base_address_2 + 1
  i2cComm->SendByte(0xcf, fBranch);            // choose to read high threshold
  i2cComm->CloseBranch(fBranch);

  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf1, fBranch);            // base_address_1 + 1
  i2cComm->SendByte(fAddress + 0x01, fBranch); // base_address_2 + 1
  i2cComm->RecvByte(fBranch, i2cStatus);       // recv 0x00;
  data = i2cComm->RecvLastByte(fBranch, i2cStatus);
  i2cComm->CloseBranch(fBranch);

  return data;
}


unsigned short Tpm98::GetTestPulseAmplitude(){
  unsigned short i2cStatus = 0;
  unsigned short data;
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch);            // base_address_1 + 0
  i2cComm->SendByte(fAddress + 0x01, fBranch); // base_address_2 + 1
  i2cComm->SendByte(0xaf, fBranch);            // choose to read tpa
  i2cComm->CloseBranch(fBranch);

  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf1, fBranch);            // base_address_1 + 1
  i2cComm->SendByte(fAddress + 0x01, fBranch); // base_address_2 + 1
  i2cComm->RecvByte(fBranch, i2cStatus);       // recv 0x00;
  data = i2cComm->RecvLastByte(fBranch, i2cStatus);
  i2cComm->CloseBranch(fBranch);

  return data;
}

unsigned short Tpm98::ElectricalTest(){
  unsigned short i2cStatus = 0;
  unsigned short response = 0x0000;
  // back:
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf1, fBranch);            // base_address_1 + 1
  i2cComm->SendByte(fAddress + 0x03, fBranch); // base_address_2 + 3
  response =  i2cComm->RecvByte(fBranch, i2cStatus);    // chn 15...8
  DEBUG_PRINT_E("Response 1st Byte: 0x" << hex << response << dec );
  //  response =  (response << 8);
  response =  i2cComm->RecvByte(fBranch, i2cStatus);    // chn 15...8
  DEBUG_PRINT_E("Response 2nd Byte: 0x" << hex << response << dec );
  response =  i2cComm->RecvByte(fBranch, i2cStatus);    // chn 15...8
  DEBUG_PRINT_E("Response 3rd Byte: 0x" << hex << response << dec );
  response =  i2cComm->RecvLastByte(fBranch, i2cStatus);    // chn 15...8
  DEBUG_PRINT_E("Response 4th Byte: 0x" << hex << response << dec );
  //  getchar();
  // goto back;

  //  response += ( 0x00ff & i2cComm->RecvByte(fBranch, i2cStatus) );
  //  return response;
  return 0;
}

int Tpm98::SetPulseWidth(unsigned short width){
  unsigned long mask_n_duration;
  unsigned long mask_pattern;
  unsigned long width_pattern;
  mask_n_duration = GetMaskNDuration();  
  mask_pattern = mask_n_duration & 0x0f0f0f0f;
  width = width & 0xf;
  width_pattern = (width << 4) + (width << 12) + (width << 20) + (width << 28);
  mask_n_duration = mask_pattern | width_pattern;
  SetMaskNDuration(mask_n_duration);
  return 0;
}


int Tpm98::SetMask(unsigned short mask){
  unsigned long mask_n_duration;
  unsigned long mask_pattern;
  unsigned long width_pattern;
  mask_n_duration = GetMaskNDuration();  
  width_pattern = mask_n_duration & 0xf0f0f0f0;
  mask_pattern = (mask & 0x000f) + ( (mask & 0x00f0) << 4 )
    + ( (mask & 0x0f00) << 8 )   + ( (mask & 0xf000) << 12 ); 
  mask_n_duration = mask_pattern | width_pattern;
  SetMaskNDuration(mask_n_duration);
  return 0;
}


unsigned short Tpm98::GetPulseWidth(){
  unsigned long mask_n_duration;
  mask_n_duration = GetMaskNDuration();  
  return (mask_n_duration & 0xf0) >> 4 ;  // taken from channels 1..4
}


unsigned short Tpm98::GetMask(){
  unsigned long mask_pattern;
  unsigned short mask;
  mask_pattern = 0x0f0f0f0f & GetMaskNDuration();
  mask = (mask_pattern & 0x0000000f) + ( (mask_pattern & 0x00000f00) >> 4 )
    + ( (mask_pattern & 0x000f0000) >> 8 ) 
    + ( (mask_pattern & 0x0f000000) >> 12);
  return mask;
}


int Tpm98::SetMaskNDuration(unsigned long DWord)
{
  char Byte;
  unsigned long Mask = 0xff000000;
  i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0, fBranch); 	 	// base_address_1 + 0
  i2cComm->SendByte(fAddress + 0x02, fBranch);	// base_address_2 + 2

  for (int t=3; t>=0; t--){
    Byte = (DWord & Mask) >> (t*8);
    i2cComm->SendByte(Byte, fBranch);
    Mask = Mask >> 8;
  }

  i2cComm->CloseBranch(fBranch);
  return 0;
}
unsigned long Tpm98::GetMaskNDuration()
{
  unsigned short i2cStatus = 0;
  unsigned long data;

  unsigned long DWord = 0;

  // i2cComm->ResetBranch(fBranch);
  i2cComm->OpenBranch(fBranch);
  i2cComm->SendByte(0xf0 + 0x01, fBranch);     // base_address_1 + 1
  i2cComm->SendByte(fAddress + 0x02, fBranch); // base_address_2 + 2

  for (int t=3; t>=1; t--){
    data = i2cComm->RecvByte(fBranch, i2cStatus) << (t*8);
	DWord= DWord | data;
  }

  data = i2cComm->RecvLastByte(fBranch, i2cStatus);
  DWord= DWord | data;
  i2cComm->CloseBranch(fBranch);

  return DWord;
}


unsigned short Tpm98::GetBranchNo(){
  return fBranch;
}

unsigned short Tpm98::GetDeviceAddress(){
  return fAddress >> 2;
}

int Tpm98::SetBranchNo(unsigned short branch_no){            // experimental / not implemented
  fBranch = branch_no;
  SetAdcReference(ADC_REFERENCE);          
  return 0;
}
int Tpm98::SetDeviceAddress(unsigned short device_address){
  fAddress = 0xff & (device_address << 2);  
  SetAdcReference(ADC_REFERENCE);         
  return 0;
}

Tpm98::~Tpm98(){
  delete i2cComm;
}

// ----------------------------------- i2c Comm ---------------------------
// ----------------------------------- i2c Comm ---------------------------

Ti2cComm::Ti2cComm(unsigned long device_base, int no_mapping){
  DEBUG_PRINT_S("Constructor Ti2cComm called");
  if (no_mapping){
    fBaseAddress = device_base;
    vme = new TVME(device_base, 0, 0);
  } else {
    fBaseAddress = device_base;
    vme = new TVME(fBaseAddress, 0x1000, VME_A16_U); // Baya 0xff);
  }
}

short Ti2cComm::ResetBranch(unsigned short branch_no){
  short CSR;
  vme->Write(CSRaddr(branch_no), 0x08, VME_D16);
  usleep(I2C_DLY); // 01. Jul. 03
  CSR = vme->Read( CSRaddr(branch_no), VME_D16);
  DEBUG_PRINT_S("Reset Branch. Status: 0x" << hex << CSR << dec);
  if (VERBOSE) if ((CSR & 0xff) != 0x01){
      // Baya cerr << "I2C: Reset branch " << branch_no << " failed.\n"; //<----------- BAYA 
      //Baya cerr << "     Status: 0x" << hex << CSR << dec << endl;     ///<<--------- BAYA
      printf( "I2C: Reset branch %d failed \n", branch_no);              //<----------- BAYA 
      DEBUG_PRINT_S ("     Status: 0x" << hex << CSR << dec);           ///<<--------- BAYA
  }
  return CSR;
}

short Ti2cComm::OpenBranch(unsigned short branch_no){
  short CSR;
  vme->Write(CSRaddr(branch_no), 0x60, VME_D16);
  //  cout << "Open Branch. RETURN"; getchar();
  usleep(I2C_DLY);
  CSR = vme->Read(CSRaddr(branch_no), VME_D16);
  //  cout << "RETURN"; getchar();
  DEBUG_PRINT_S("Open Branch. Status: 0x" << hex << CSR << dec);
  if (VERBOSE)  if ((CSR & 0xff) != 0x61){
      /// BAYA cerr << "I2C: Open Branch No." << branch_no << " error detected.\n";  //<----------- BAYA 
      // BAYA cerr << "     Status: 0x" << hex << CSR << dec << endl;  ///<<--------- BAYA
      printf( "I2C: Open Branch No %d error detected \n", branch_no);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Status: 0x" << hex << CSR << dec);             //<----------- BAYA 
  }
  return CSR;
}

short Ti2cComm::CloseBranch(unsigned short branch_no){
  short CSR;
  vme->Write(CSRaddr(branch_no), 0x00, VME_D16);
  //  cout << "Close Branch RETURN"; getchar();
  usleep(I2C_DLY);
  CSR = vme->Read(CSRaddr(branch_no), VME_D16);
  //  cout << "RETURN"; getchar();
  DEBUG_PRINT_S("Close Branch. Status: 0x" << hex << CSR << dec);
  if (VERBOSE) if ( ((CSR & 0xff) != 0x61)  && ((CSR & 0xff) != 0x01) ) {
      // BAYA cerr << "I2C: Close Branch " << branch_no << " error detected.\n";    //<----------- BAYA
      /// BAYA  cerr << "     Status: 0x" << hex << CSR << dec << endl;              //<----------- BAYA
      printf( "I2C: Close Branch %d error detected \n", branch_no);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Status: 0x" << hex << CSR << dec);             //<----------- BAYA 
  }
  return CSR;
}

short Ti2cComm::SendByte(unsigned short data, unsigned short branch_no){
  short CSR;
  vme->Write(DTAaddr(branch_no), data, VME_D16);
  //  cout << "Send byte RETURN"; getchar();
  vme->Write(CSRaddr(branch_no), 0xe0, VME_D16);
  usleep(I2C_DLY);
  //  cout << "RETURN"; getchar();
  CSR = vme->Read(CSRaddr(branch_no),VME_D16);
  //  cout << "RETURN"; getchar();
  DEBUG_PRINT_S("Send Byte: 0x" << hex << data << ", Status: 0x" << CSR << dec);
  if (VERBOSE) if ((CSR & 0xff) != 0x61){
      // BAYA  cerr << "I2C: TransferByte " << branch_no << " error detected\n";     //<----------- BAYA
      /// BAYA  cerr << "     Status: 0x" << hex << CSR << dec << endl;         //<----------- BAYA
      printf( "I2C: TransferByte %d error detected \n", branch_no);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Status: 0x" << hex << CSR << dec);             //<----------- BAYA 
  }
  return CSR;
}
 
unsigned short Ti2cComm::RecvByte(unsigned short branch_no, unsigned short &status){
  unsigned short data;
  vme->Write(CSRaddr(branch_no), 0xc0, VME_D16);
  //  cout << "Recv. Byte: RETURN"; getchar();
  usleep(I2C_DLY);
  status = vme->Read(CSRaddr(branch_no), VME_D16);
  //  cout << "RETURN"; getchar();
  DEBUG_PRINT_S("Receive Byte. Prepare, status: 0x" << hex << status << dec);
  if ( ((status & 0xff) != 0x61) && ((status & 0xff) != 0x49) ){
    // BAYA cerr << "I2C: RecvByte " << branch_no << " error detected.\n";        //<----------- BAYA
    // BAYA cerr << "     Status: 0x" << hex << status << dec << ".\n";        //<----------- BAYA
    // BAYA cerr << "     Trying to go on... " << endl;                        //<----------- BAYA  
      printf( "I2C: RecvByte %d error detected \n", branch_no);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Status: 0x" << hex << CSR << dec);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Trying to go on ...  \n ");                //<----------- BAYA 
  }
  data = 0xaa;                // you may take this out...
  data = 0xff &  vme->Read(DTAaddr(branch_no), VME_D16);
  //  cout << "RETURN"; getchar();
  DEBUG_PRINT_S("Receive Byte. Got: 0x" << hex << data << dec);
  return data;
}

unsigned short Ti2cComm::RecvLastByte(unsigned short branch_no, unsigned short &status){
  // short CSR;
  unsigned short data;
  vme->Write(CSRaddr(branch_no), 0xd0, VME_D16);
  usleep(I2C_DLY);
  status = vme->Read(CSRaddr(branch_no),VME_D16);
  DEBUG_PRINT_S("Receive Last Byte. Prepare, status: 0x" << hex << status << dec);
  if (VERBOSE) if ((status & 0xff) != 0x61){
      // BAYA cerr << "I2C: RecvLastByte " << branch_no << " error detected.\n";        //<----------- BAYA
      // BAYA  cerr << "     Status: 0x" << hex << status << dec << ".\n";          //<----------- BAYA
      //  BAYA cerr << "     Trying to go on... " << endl;                        //<----------- BAYA  
      printf( "I2C: RecvLasByte %d error detected \n", branch_no);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Status: 0x" << hex << CSR << dec);             //<----------- BAYA 
      DEBUG_PRINT_S ("     Trying to go on ...  \n ");                //<----------- BAYA 
  }
  data = 0xaa;
  data = 0xff & vme->Read(DTAaddr(branch_no), VME_D16);
  DEBUG_PRINT_S("Receive Last Byte. Got: 0x" << hex << data << dec);
  return data;
}

short Ti2cComm::CSRaddr(unsigned short BranchNo){
  //  cout << "-- CSRaddr(" << BranchNo << "):" 
  //     << ((BranchNo < 8) ? 0x44+ 0x08 * BranchNo : -1) << endl;
  return (BranchNo < 8) ? 0x44+ 0x08 * BranchNo : -1;
}

short Ti2cComm::DTAaddr(unsigned short BranchNo){
  return (BranchNo < 8) ? 0x40 + 0x08 * BranchNo: -1;
}

Ti2cComm::~Ti2cComm(){
  DEBUG_PRINT_S("Destructor Ti2cComm called");
  delete vme;
}
