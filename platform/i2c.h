
#ifndef I2C_H
#define I2C_H
#include <stdint.h>

void i2c1_init(void);
void i2c1_read (uint8_t saddr, uint16_t n, uint8_t* data);
void i2c1_write (uint8_t saddr, uint16_t n, const uint8_t* data);

#endif