/******************************************************************************
 *                                                                             *
 *                                                                             *
 * FIADC VME module programm                                                 *
 *                                                                             *
 ******************************************************************************/


#include <stdlib.h>
#include "stdio.h"
#include "time.h"
//extern "C" {
#include "i2c.h"
#include "vmemapper.h"
//}

int main(int argc, char **argv) {
  int read_val, hard_wired_id, port_id,ext_port_id;

  if(argc <2) {
    printf("To few arguments specified\n");
    exit(1);
  }
  ++argv;
  sscanf(*argv, "%x", &vme_addr);
  ext_port_id=-1;
  if(argc==3) {
    ++argv;
    sscanf(*argv, "%x", &ext_port_id);
  }

  calc_address();
  i2c_reset();       

  for(port_id=7;port_id>0;port_id--){
    if(ext_port_id>=0&&ext_port_id<8)
      port_id=ext_port_id;
    read_val = ntohl(*vmeSCR);
    if ( (read_val & (1 << (port_id+8))) == 0 ) {
      printf( "ADC port %u is not attached!, SCR = %x\n", port_id, read_val);
      goto next;
    }

    set_port(port_id,0xff);
    //----------------------
    // READ BACK HARDWIRED ID
    //----------------------
    read_from_i2c(1,0,&hard_wired_id);
    //-----------------------
    // write new ID to ID register
    //-----------------------
    write_to_i2c(2,1,port_id,hard_wired_id,0,0);
    //----------------------
    // READ BACK prog ID
    //----------------------
    read_from_i2c(1,1,&read_val);
    printf("Port: %2x, HardwareID: %2x, GeoID: %2x\n", port_id, hard_wired_id, read_val & 0xff);
next:
    if(ext_port_id>=0&&ext_port_id<8)
      break;
  }

  /* End of processing */

  /* Unmap the memory window */
  UnmapVME();
  return 0;
}
