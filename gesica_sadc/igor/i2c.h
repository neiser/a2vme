#ifndef _I2C_H_ 
#define _I2C_H_
#include "stdio.h"
#include "time.h"
#include "vmemapper.h"


void calc_address(void);
void i2c_reset(void);
void set_port(const int setPort, const int setAdc_id);
int write_to_i2c(const int length, const int address, int value_0, int value_1, int value_2, int value_3);
int read_from_i2c(const int length, const int address, int *read_val);
int select_msadc(int);
int read_msadc (const int address, int *read_val);
int write_msadc (const int address, const int data0, const int data1);
int i2c_read(int, int);
int i2c_read2(int, int, int*);
int i2c_write(int, int, int);



/**/
int vme_addr;

unsigned short *vi2cCR;
unsigned short *vi2cSR;
unsigned short *vi2cDT;
unsigned short *vi2cDTh;
unsigned short *vi2cRD;
unsigned short *vi2cRDh;
unsigned long  *vmeSCR;
unsigned long  *vmeDAT;
unsigned short *vmeSW;
unsigned long  *vmeI2C_PORT;

int   i2c_addr;
// int port, adc_id, i2c_addr;
int i2c_value, i2c_value_0, i2c_value_1, i2c_value_2, i2c_value_3;
int i2c_length;

char i2c_op;
int debug;
/**/
    unsigned long vmeWinOffset, vmeWinSize;

    unsigned long *vmeWinAddr;

    unsigned int genVmeBaseAddr;

    unsigned int genBaseAddr;


    int value;
    int n;
    int i;
    int dd;




#endif
