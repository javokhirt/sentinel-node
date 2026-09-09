#include "i2c.h"
#include <stdint.h>

char saddr = 0x44; // allow 1ms of delay before


char measureHigh[2] = {0x24, 0x00}; //allow 15ms of delay for sensor to measure


// you should wait 15ms before the next command
