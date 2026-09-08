#ifndef I2C_H
#define I2C_H

void I2C1_Init(void);
void I2C1_byteRead (char saddr, char maddr, char* data);
void I2C1_burstRead (char saddr, char maddr, int n, char* data);
void I2C1_burstWrite (char saddr, char maddr, int n, char* data);

#endif