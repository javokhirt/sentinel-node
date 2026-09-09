#include "i2c.h"
#include <stdint.h>

// commands --> you should wait 15ms before the next command
char measureHigh[2] = {0x24, 0x00};


// you should wait 15ms before the next command