// #include "vmemapper.h"
#include <stdlib.h>
#include "i2c.h"


  void calc_address(){
    if(vme_addr < 0x100)
      vme_addr = (vme_addr << 16) & 0xff0000;
    else if((vme_addr < 0x1000) & (vme_addr > 0xff))
      vme_addr = (vme_addr << 12) & 0xfff000;
    else printf("VME ADDRESS is OUT OF RANGE\n");

    vmeWinOffset = vme_addr; /* Start from the bottom */
    vmeWinSize   = 0x100;    /* Map part of the 24-bit address the VME space */

    //     struct vme_mapping_ctrl mapStruct;
    struct vme_mapping_ctrl mapStruct = {
      VMEMAP_DWIDTH_32,      /* set data width: 16 bits at a time */
      VMEMAP_ASPACE_A24,     /* set address space: 24 bit addresses */
      VMEMAP_SUPUSERAM_USER, /* set privilege mode: access as user */
      VMEMAP_PRGDATAAM_DATA  /* set access type: access data, not program code */
    };

    /* Map the VME window onto a memory window.
       A pointer to the first 32-bit word is returned */
    vmeWinAddr = (unsigned long *)MapVME(vmeWinOffset, vmeWinSize, mapStruct);

    /* Set the VME base address of the device.
       It depends on a switch setting on the board */
    genVmeBaseAddr = vme_addr;

    /* Calculate the memory base address of the device */
    genBaseAddr = (unsigned int)vmeWinAddr + 
      genVmeBaseAddr - (unsigned int)vmeWinOffset;

    //    printf("genBaseAddr: %x\n", genBaseAddr);

    /* Calculate the pointer to 16-bit registers */
    vi2cCR  = (unsigned short *) (genBaseAddr + 0x8  + 0x40);  
    vi2cSR  = (unsigned short *) (genBaseAddr + 0xc  + 0x40);
    vi2cDT  = (unsigned short *) (genBaseAddr + 0x0  + 0x40);
    vi2cDTh = (unsigned short *) (genBaseAddr + 0x14 + 0x40);
    vi2cRD  = (unsigned short *) (genBaseAddr + 0x4  + 0x40);
    vi2cRDh = (unsigned short *) (genBaseAddr + 0x18 + 0x40);

    vmeSCR = (unsigned long *) (genBaseAddr + 0x20);  
    vmeDAT = (unsigned long *) (genBaseAddr + 0x34);
    vmeSW  = (unsigned short *) (genBaseAddr + 0x50);
    vmeI2C_PORT = (unsigned long *) (genBaseAddr + 0x2c);

    /* set active port for I2C operation */
    //     *vmeI2C_PORT = htonl(port);
    //     *vmeSW = htonl(adc_id);

  }


void i2c_reset(void) {
  //printf("i2c reset \n");
  *vi2cCR = htons(0x40);
  while((ntohs(*vi2cSR) & 1) == 1);
}


void set_port(const int setPort, const int setAdc_id) {
  /* set active port for I2C operation */
  *vmeI2C_PORT = htonl((setPort) & 0xff);
  /* set ADC_ID for I2C operation */
  *vmeSW = htons(setAdc_id);
}


int write_to_i2c(const int length, const int address, int value_0, int value_1, int value_2, int value_3) {
  /* write to i2c register */
  i2c_value = ((value_1 << 8) & 0xff00) + (value_0 & 0xff);
  *vi2cDT = htons(i2c_value);
  i2c_value = ((value_3 << 8) & 0xff00) + (value_2 & 0xff);
  *vi2cDTh = htons(i2c_value);

  /* i2c operation write two bytes */
  if (length == 1) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x84);
  if (length == 2) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x88);   
  if (length == 3) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x8c);
  if (length == 4) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x90);


  /* wait for completing I2C operation */
  int read_val;
  read_val = 1;
  while((read_val & 0x1) == 1) read_val = ntohs(*vi2cSR);
  if((read_val & 0x7f) != 0x30) {
    //printf("I2C WRITE Error. Status %2x \n", read_val & 0x7f);
    read_val = -read_val;
    return -1;
  }
  else //if(debug)
    //printf("I2C WRITE OK\n");
  return read_val;
}


int read_from_i2c(const int length, const int address, int *read_val) {
  /* read VALUE from I2C register */
  if(length == 1) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x94);
  if(length == 2) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x98);
  if(length == 3) *vi2cCR = htons(((address<<8) & 0x7f00) + 0x9c);
  if(length == 4) *vi2cCR = htons(((address<<8) & 0x7f00) + 0xa0);
  *read_val = ntohs(*vi2cCR);
  //if(debug)
    //printf("I2C command execution 0x%x\n", *read_val);

  /* wait for completing I2C operation */
  *read_val = 1;
  while((*read_val & 0x1) == 1) *read_val = ntohs(*vi2cSR);
  if((*read_val & 0xf8) != 0x50){ 
    printf("I2C READ Error. Status %x \n", *read_val & 0x7f);    
    return 1;
  }
  if(length==1)  *read_val= (ntohs(*vi2cRD) + (ntohs(*vi2cRDh)<<16) )&0xff;
  else if(length==2)  *read_val= (ntohs(*vi2cRD) + (ntohs(*vi2cRDh)<<16) )&0xffff;
  else if(length==3)  *read_val= (ntohs(*vi2cRD) + (ntohs(*vi2cRDh)<<16) )&0xffffff;
  else if(length==4)  *read_val= (ntohs(*vi2cRD) + (ntohs(*vi2cRDh)<<16) )&0xffffffff;
  else return 2;
  return 0; 
}


int select_msadc(int chip) {
  if(chip==-1) chip=1;
  else chip<<=1;
  return write_to_i2c(1,0xd,chip,0,0,0);
}

int read_msadc (const int address,  int *read_val) {

  int full_address = 0x40 | (address & 0x1f);
  int byte0 = (address >> 5) & 0xff;

  write_to_i2c(1, full_address, byte0,0,0,0);
  return read_from_i2c(2, full_address, read_val);	

}


int write_msadc (const int address, const int data0, const int data1){
  int full_address;
  int byte0, byte1, byte2; 

  full_address = 0x40 | (address & 0x1f);

  byte0 = (address >> 5) & 0xff;
  byte1 = data0;
  byte2 = data1;

  write_to_i2c(3, full_address, byte0, byte1, byte2,0);

  return 0;
}


int i2c_read(int i2c_addr, int i2c_length)
{
  int vme_value;
  while((ntohs(*vi2cSR) & 1) == 1);
  if(i2c_length == 1) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x94);
  if(i2c_length == 2) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x98);
  if(i2c_length == 3) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x9c);
  if(i2c_length == 4) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0xa4);
  vme_value = ntohs(*vi2cSR);
  while((vme_value & 0x1) == 1) vme_value = ntohs(*vi2cSR);
  if((vme_value & 0x7f) != 0x54) vme_value = - vme_value;
  else vme_value = ntohs(*vi2cRD) + ((ntohs(*vi2cRDh) << 16) & 0xffff0000);

  return vme_value;
}

// int i2c_write(int i2c_addr, int i2c_length, int i2c_data)
// {
//   int vme_value;
//   while((ntohs(*vi2cSR) & 1) == 1);
//   *vi2cDT = htons(i2c_data & 0xffff);
//   *vi2cDTh = htons((i2c_data >> 16) & 0xffff);
//   if(i2c_length == 1) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x84);
//   if(i2c_length == 2) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x88);
//   if(i2c_length == 3) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x8c);
//   if(i2c_length == 4) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x90);
//   vme_value = ntohs(*vi2cSR);
//   while((vme_value & 0x1) == 1) vme_value = ntohs(*vi2cSR);
//   if((vme_value & 0x7f) != 0x34) vme_value = - vme_value;
//   return vme_value;
// }
// 
int i2c_write(int i2c_addr, int i2c_length, int i2c_data)
{
  time_t t1, t2;
  int count=0;
  int vme_value;

  //while((ntohs(*vi2cSR) & 1) == 1);
  *vi2cDT = htons(i2c_data & 0xffff);
  *vi2cDTh = htons((i2c_data >> 16) & 0xffff);
  if(i2c_length == 1) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x84);
  if(i2c_length == 2) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x88);
  if(i2c_length == 3) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x8c);
  if(i2c_length == 4) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x90);
  vme_value = ntohs(*vi2cSR);

  t1 = time((time_t *) NULL);
  while((vme_value & 0x1) == 1){
    vme_value = ntohs(*vi2cSR);
    usleep(10);
    //wait for ready

    count++;
    if (count == 10000000){
      printf(" i2c_write waiting for i2c ready ... (%x)\n", vme_value);
      count=0;

      t2 = time((time_t *) NULL);
      if (difftime(t2, t1) > 10.){
        printf("\a no i2c ready for 10 seconds (%x)\n", ntohs(*vi2cSR));
        // ------- I2C RESET ----------------------------------- 
        *vi2cCR = htons(0x40);
        exit(-1);
      }
    }
  }

  if((vme_value & 0x78) != 0x30) vme_value = - vme_value;
  return vme_value;
}


int i2c_read2(int i2c_addr, int i2c_length, int* data)
{
  time_t t1, t2;
  int count=0;
  int vme_value;
  //while((ntohs(*vi2cSR) & 1) == 1);
  if(i2c_length == 1) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x94);
  if(i2c_length == 2) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x98);
  if(i2c_length == 3) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0x9c);
  if(i2c_length == 4) *vi2cCR = htons(((i2c_addr<<8) & 0x7f00) + 0xa0);
  vme_value = ntohs(*vi2cSR);

  t1 = time((time_t *) NULL);
  while((vme_value & 0x1) == 1){
    vme_value = ntohs(*vi2cSR);
    usleep(10);
    //wait for ready
    count++;
    if (count == 10000000){
      printf(" i2c_read waiting for i2c ready ... (%x)\n", vme_value);
      count=0;

      t2 = time((time_t *) NULL);
      if (difftime(t2, t1) > 10.){
        printf("\a no i2c ready for 10 seconds (%x)\n", ntohs(*vi2cSR));
        // ------- I2C RESET ----------------------------------- 
        *vi2cCR = htons(0x40);
        exit(-1);
      }
    }
  }

  if((vme_value & 0x78) != 0x50) vme_value = - vme_value;
  else {
    *data = ntohs(*vi2cRD) + ((ntohs(*vi2cRDh) << 16) & 0xffff0000);
    return 0;
  }

  return vme_value;
}




