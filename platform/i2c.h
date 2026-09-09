#ifndef I2C_H
#define I2C_H

void I2C1_Init(void);
void i2c1_read (uint8_t saddr, int n, char* data);
void i2c1_write (uint8_t saddr, int n, char* data);

#endif