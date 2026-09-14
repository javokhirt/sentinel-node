#include "systick.h"
#include "stm32f411xe.h"

/* 1 ms at 16 MHz processor clock: LOAD = f_clk / 1000 */
#define SYSTICK_LOAD_VAL         16000
#define CTRL_ENABLE               (1U<<0)
#define CTRL_CLKSOURCE            (1U<<2)   /* 1 = processor clock */
#define CTRL_COUNTERFLAG          (1U<<16)  /* set on 0; clears when CTRL is read */


void systick_delay_ms(int delay_ms) {
    SysTick->LOAD = SYSTICK_LOAD_VAL;
    SysTick->VAL = 0;
    SysTick->CTRL = CTRL_ENABLE | CTRL_CLKSOURCE;

    for (volatile int i=0; i<delay_ms; i++) {
        while ((SysTick->CTRL & CTRL_COUNTERFLAG) == 0){
        }
    }

    SysTick->CTRL = 0;
}
