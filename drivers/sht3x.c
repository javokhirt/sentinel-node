#include <stdint.h>
#include <stddef.h>

#include "sht3x.h"
#include "sht3x_decode.h"

#include "i2c.h"
#include "systick.h"

#define SHT3X_ADDR              0x44U

/* 16-bit commands, sent MSB first. Tables 9, 13, 14, 16, 17, 19. */
#define CMD_MEAS_HIGH_NOSTRETCH 0x2400U
#define CMD_SOFT_RESET          0x30A2U
#define CMD_READ_STATUS         0xF32DU
#define CMD_CLEAR_STATUS        0x3041U
#define CMD_HEATER_ON           0x306DU
#define CMD_HEATER_OFF          0x3066U

#define MEAS_TIME_HIGH_MS       15U     /* high-repeatability conversion, Table 5 */
#define RESET_TIME_MS           2U      /* 1.5 ms max, rounded up */


static sht3x_status_t send_cmd(uint16_t cmd)
{
    uint8_t buf[2];

    buf[0] = (uint8_t)(cmd >> 8);
    buf[1] = (uint8_t)(cmd & 0xFFU);

    i2c1_write(SHT3X_ADDR, 2U, buf);

    return SHT3X_OK;
}

sht3x_status_t sht3x_init(void)
{
    sht3x_status_t st = send_cmd(CMD_SOFT_RESET);
    if (st != SHT3X_OK) {
        return st;
    }

    systick_delay_ms(RESET_TIME_MS);
    return SHT3X_OK;
}

sht3x_status_t sht3x_read_status(uint16_t *status)
{
    uint8_t buf[3];     /* 2 data bytes + 1 CRC */

    sht3x_status_t st = send_cmd(CMD_READ_STATUS);
    if (st != SHT3X_OK) {
        return st;
    }

    i2c1_read(SHT3X_ADDR, 3U, buf);

    if (sht3x_crc8(&buf[0], 2U) != buf[2]) {
        return SHT3X_ERR_CRC;
    }

    if (status != NULL) {
        *status = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    }
    return SHT3X_OK;
}

sht3x_status_t sht3x_read(float *temp_c, float *rh)
{
    sht3x_status_t st = send_cmd(CMD_MEAS_HIGH_NOSTRETCH);
    if (st != SHT3X_OK) {
        return st;
    }

    systick_delay_ms    (MEAS_TIME_HIGH_MS);

    /* 6 bytes: T MSB, T LSB, CRC, RH MSB, RH LSB, CRC. Section 4.4. */

    uint8_t buf[6];
    i2c1_read(SHT3X_ADDR, 6U, buf);

    if (sht3x_crc8(&buf[0], 2U) != buf[2]) {
        return SHT3X_ERR_CRC;
    }
    if (sht3x_crc8(&buf[3], 2U) != buf[5]) {
        return SHT3X_ERR_CRC;
    }

    uint16_t raw_t  = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    uint16_t raw_rh = (uint16_t)(((uint16_t)buf[3] << 8) | buf[4]);


    if (temp_c != NULL) {
        *temp_c = -45.0f + 175.0f * ((float)raw_t / 65535.0f);
    }
    if (rh != NULL) {
        *rh = 100.0f * ((float)raw_rh / 65535.0f);
    }

    return SHT3X_OK;
}