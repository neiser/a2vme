//g++ -Wall prog_pm98.cc vme++.o lpm98.o -o prog_pm98 -L/lib/ces -lvme


/* Changes:
   - Feb.  1st 04: -dk- "nicified" output file (a bit)
   - Mai.  6th 04: -dk- made output to file even nicer :). 
                        Added header and timestamp.//
//
// Update     B.Ousena 29 Sep. 2012 ....  Start update code to run on Linux ..

*/

#include <iostream>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vme++.h"
#include "lpm98.h"

using namespace std;


int main(int argc, char *argv[]){
  unsigned short wADCRef;
  unsigned short wBranch, wDevice;
  unsigned short wThresLo, wThresHi;
  unsigned short wValueLo, wValueHi;
  unsigned short wMask, wValueMask;
  unsigned int dwLine = 0;
  time_t the_time;
  int bReadOnly = 0;
  char szLine[256];
  char szFile[256];

  TVME vme(0xf800, 0x1000,  VME_A16_U);
  Tpm98 disc(vme.GetVirtBase(), 0, 0);

  FILE* fpSetup;

  if (argc<2)
    sprintf(szFile, "pm98.set");
  else
    {
      if(!strcmp(argv[1], "-r"))
	{
	  sprintf(szFile, "pm98.set");
	  bReadOnly = 1;
	}
      else
	{
	  sprintf(szFile, "%s", argv[1]);
	  bReadOnly = 0;
	}
    }

  if(bReadOnly)
    cout << endl << "# Reading Discriminators specified in file " << szFile << endl;
  else
    cout << endl << "# Programming from file " << szFile << endl;

  (void)time(&the_time);
  fpSetup = fopen(szFile, "r");
  if(fpSetup == NULL) {
    cout << "# Could not open " << szFile << ", exiting." << endl;
    exit(1);
  }
  printf("# Device\t  Thr (low)\t Thr (high)\t    Mask\n");
  printf("# Br  Dev\t set  read\t set  read \t  set     read\n");
  printf("# Setting/checking discriminators @ %s", 
	  ctime(&the_time));
  if(bReadOnly) 
    printf("# comparing current settings with file: %s\n#\n", szFile);
  else
    printf("# setting up discriminators according to file: %s#\n\n",
	    szFile);
  printf("TimeStamp: %lu\n", (unsigned long)the_time);
  printf("# Device\t  Thr (low)\t Thr (high)\t    Mask\n");
  printf("# Br  Dev\t set  read\t set  read \t  set     read\n");
  while (!feof(fpSetup)){
    dwLine++;
    fgets(szLine, sizeof(szLine), fpSetup);
    
    if(szLine[0]!='#'){
      if(sscanf(szLine, "%hx %hd %hd %hd %hd %hx\n",
		&wADCRef, &wBranch, &wDevice, &wThresLo, &wThresHi, &wMask)
	 !=6){
	cout << endl << "Wrong parameter count in line " 
	     << dwLine << ". Exiting." << endl;
	fclose(fpSetup);
	return -1;
      }
	  
      disc.SetBranchNo(wBranch);         // Choose device
      disc.SetDeviceAddress(wDevice);
      disc.SetAdcReference(wADCRef);
      if (!bReadOnly){                   // Set values
	disc.SetLowThreshold(wThresLo);
	disc.SetHighThreshold(wThresHi);
	disc.SetMask(wMask);
      }
      wBranch = disc.GetBranchNo();
      wDevice = disc.GetDeviceAddress();
      wValueLo = disc.GetLowThreshold();  // Check values
      wValueHi = disc.GetHighThreshold();
      wValueMask = disc.GetMask();

      printf("  %2d %3d\t %3d  %3d \t %3d  %3d  \t 0x%-4x  0x%-4x\n",
	     wBranch, wDevice, wThresLo, wValueLo, wThresHi, wValueHi, 
	     wMask, wValueMask);
    }
  }
  fclose(fpSetup);
  cout << endl << "Programming from file " << szFile << " completed" << endl;
  return 0;
}
