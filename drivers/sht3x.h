#ifndef SHT3X_H
#define SHT3X_H

#include <stdint.h>

/* SHT3x-DIS temperature and humidity sensor, I2C.
 *
 * Datasheet: Sensirion SHT3x-DIS, December 2022, Version 7.
 * Section references in this driver refer to that document.
 *
 * Covers SHT30 / SHT31 / SHT35 — the protocol is identical across all
 * three, only the factory accuracy grade differs.
 *
 * Depends on:
 *   - platform/i2c.c  (i2c1_read / i2c1_write)
 *   - a millisecond systick delay (systick_delay_ms)
 */

typedef enum {
    SHT3X_OK = 0,
    SHT3X_ERR_CRC,      // data arrived but the checksum did not match
    SHT3X_ERR_I2C,      // bus level failure — not yet reachable, see note in .c
} sht3x_status_t;


sht3x_status_t sht3x_init(void);
sht3x_status_t sht3x_read(float *temp_c, float *rh);
sht3x_status_t sht3x_read_status(uint16_t *status);

#endif