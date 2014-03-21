#ifndef GESICA_LIB_H
#define GESICA_LIB_H

#include <vector>

typedef unsigned long UInt_t;
typedef volatile UInt_t* vme32_t; 

bool i2c_wait(vme32_t gesica, bool check_ack);

bool i2c_reset(vme32_t gesica);

void i2c_set_port(vme32_t gesica, UInt_t port_id, bool broadcast = false);

bool i2c_read(vme32_t gesica, UInt_t bytes, UInt_t addr, UInt_t& data);

bool i2c_write(vme32_t gesica, UInt_t bytes, UInt_t addr, UInt_t data);

bool i2c_read_reg(vme32_t gesica, UInt_t adc_side, UInt_t reg, UInt_t& data);

bool i2c_write_reg(vme32_t gesica, UInt_t adc_side, UInt_t reg, UInt_t data);

bool load_rbt(const char* rbt_filename, std::vector<UInt_t>& data);

bool init_gesica(vme32_t gesica, 
                 std::vector<UInt_t>& ports, 
                 bool skip_i2c = false);

#endif // GESICA_LIB_H
