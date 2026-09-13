#ifndef SHT3X_DECODE_H
#define SHT3X_DECODE_H

#include <stdint.h>
#include <stdbool.h>

uint8_t sht3x_crc8(const uint8_t *data, uint8_t len);

float sht3x_raw_to_celsius(uint16_t raw);
float sht3x_raw_to_rh(uint16_t raw);

bool sht3x_decode_measurement(const uint8_t buf[6], float *temp_c, float *rh);

#endif 
