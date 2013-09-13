/* library to communicate with Pawel Marckiniewskis discriminators 

  lpm98 :  Dirk Krambrich, Sven Schumann, Jan 2003

  Update:  Dirk Krambrich, 22nd Apr. 2004: Swapped "low" and "high" threshold.

           This library contains basic classes to communicate with 
	   Pawel Marciniewskis discriminators. A test  environment 
	   is in tst_pm98.cc.

	   class Ti2cComm: VME-i2c-Interface.
	   class Tpm98:    The discriminator itself

	   What needs to be done:
	   * Check methods to set pattern, minimum pulse widh....
	     ->make them more individual.
	   * Not all features have been tested at by now :(
	   * Test, look especially at addressing: BranchNo,
	     ControllerAddress may not be passed through correctly

	   Things to think about:
	   * When does one have to reset the branch? Maybe this
	     method should be made available to the user of Tpm98.
	   * Is it possible to set stuff in the ringbuffer
	     indevidually?

	   Pitfalls:
	   * ADC reference has to be set before writing thresholds
	     (see manual).

	   How does it work? How do I extend it?
	   Look at Pawels manual. The program is intended to reflect
	   the steps in the manual as close as possible.

	   VME-Access:
	   This is done by using the class vme++. Look at vme++.h if
	   you want to exchange it according to your needs/philosopy.

	   How is debugging supported?
	   * Sourcecode: Look at dsm.h
	   * Hardware:   #define VERBOSE 1 (see below)

*/

#ifndef _LPM98_H_
#define _LPM98_H_

#include "vme++.h"

/* Basic class modelling the vme->i2c interface */
class Ti2cComm{
public:
  Ti2cComm(unsigned long device_base, int no_mapping = 0);  // vme base-address of
  ~Ti2cComm();                                              // i2c controller, map
  //                                                           the vme yourself 
  //                                                           (or get it externally)
  short ResetBranch(unsigned short branch_no);
  short OpenBranch( unsigned short branch_no);
  short CloseBranch(unsigned short branch_no);
  short SendByte(unsigned short data, unsigned short branch_no);
  unsigned short RecvByte(unsigned short branch_no, unsigned short &status);
  unsigned short RecvLastByte(unsigned short branch_no, unsigned short &status);
  
private:
  unsigned long fBaseAddress;
  short CSRaddr(unsigned short BranchNo);  // CSR Address of given Branch
  short DTAaddr(unsigned short BranchBo);  // Data-Register Address of Branch
  TVME* vme;
};


/* Class modelling the discriminator */
class Tpm98{
public:
  Tpm98(unsigned long controller_base,
	unsigned short device_branch, unsigned short device_address, int map_controller = 0);
  //    Parameters: vme baseaddress of i2c-controller, branch at i2c-controller,
  //                address set at device  (!not supported at the moment!)


  /* BAYA int SetParameters(unsigned short branch_no, unsigned short device_address,
		    unsigned short adc_reference = 0xff, 
		    unsigned short low_threshold = 50,  unsigned short high_threshold = 50,
		    unsigned short pulse_width   = 0x0, unsigned short mask = 0x0,
		    unsigned short output_mode   = 0);
  */

int SetParameters(unsigned short branch_no, unsigned short device_address,
		    unsigned short adc_reference, 
		    unsigned short low_threshold,  unsigned short high_threshold,
		    unsigned short pulse_width, unsigned short mask,
		    unsigned short output_mode);


  
  //  int PrintParameters(unsigned short branch_no, unsigned short device_address);
       
  // BAYA int SetAdcReference( unsigned short adc_reference = 0xff);
int SetAdcReference( unsigned short adc_reference);


  int SetLowThreshold( unsigned short low_threshold);
  int SetHighThreshold(unsigned short high_threshold);
  int SetPulseWidth(unsigned short width);              // common for all channels
  int SetMask(unsigned short mask);
  int SetTestPulseAmplitude(unsigned short amplitude);  // experimental


  unsigned short GetBranchNo();
  unsigned short GetDeviceAddress();
  int SetBranchNo(unsigned short branch_no);           // 
  int SetDeviceAddress(unsigned short device_address); // 

  unsigned short GetLowThreshold();
  unsigned short GetHighThreshold();
  unsigned short GetPulseWidth();
  unsigned short GetMask();
  unsigned short GetTestPulseAmplitude();              // experimental

  int SetMaskNDuration(unsigned long DWord);
  unsigned long GetMaskNDuration();

  unsigned short ElectricalTest();                    // send test pulse and
  // read back which channes responded. If all channels are unmasked, the
  // received byte should be 0xff unsless some channels are malfunctioning.

  ~Tpm98();
  private:
  // public:
  Ti2cComm* i2cComm;
  unsigned long  fControllerBase;    // has to be long to take virt. address
  unsigned short fBranch;
  unsigned short fAddress;
};


#endif
