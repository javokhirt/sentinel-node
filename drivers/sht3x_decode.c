#include "sht3x_decode.h"
#include <stddef.h>


uint8_t sht3x_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;

    for (uint8_t i = 0U; i < len; i++) {
        crc ^= data[i];

        for (uint8_t bit = 0U; bit < 8U; bit++) {
            if (crc & 0x80U) {
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

float sht3x_raw_to_celsius(uint16_t raw)
{
    return -45.0f + 175.0f * ((float)raw / 65535.0f);
}

float sht3x_raw_to_rh(uint16_t raw)
{
    return 100.0f * ((float)raw / 65535.0f);
}

bool sht3x_decode_measurement(const uint8_t buf[6], float *temp_c, float *rh)
{
    if (sht3x_crc8(&buf[0], 2U) != buf[2]) {
        return false;
    }
    if (sht3x_crc8(&buf[3], 2U) != buf[5]) {
        return false;
    }

    /* Casts matter: without them buf[0] << 8 is evaluated as int. */
    uint16_t raw_t  = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    uint16_t raw_rh = (uint16_t)(((uint16_t)buf[3] << 8) | buf[4]);

    if (temp_c != NULL) {
        *temp_c = sht3x_raw_to_celsius(raw_t);
    }
    if (rh != NULL) {
        *rh = sht3x_raw_to_rh(raw_rh);
    }
    return true;
}
