#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "i2c.h"
#include "uart.h"
#include "systick.h"
#include "sht3x.h"

#define SAMPLE_PERIOD_MS    1000

volatile float   g_temp_c;
volatile float   g_rh;
volatile uint32_t g_crc_errors;

int main(void)
{
    i2c1_init();
    uart2_init();

    if (sht3x_init() != SHT3X_OK) {
        while (1) {}
    }

    while (1) {
        float temp_c;
        float rh;

        sht3x_status_t st = sht3x_read(&temp_c, &rh);

        if (st == SHT3X_OK) {
            g_temp_c = temp_c;
            g_rh     = rh;
        } else {
            g_crc_errors++;
        }

        printf("Temperature: %.2f \n \r", g_temp_c);
        printf("Relative Humidity: %.1f%% \n \r", g_rh);
        
        systick_delay_ms(SAMPLE_PERIOD_MS);
    }
}
