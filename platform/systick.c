#include "systick.h"
#include "stm32f411xe.h"

#define SYSTICK_LOAD_VAL         16000 //this gives us 1ms of delay. see the formula for the systick load value.
#define CTRL_ENABLE               (1U<<0) // to enable the systick timer!!!
#define CTRL_CLKSOURCE            (1U<<2) // to use the external clock source!!!
#define CTRL_COUNTERFLAG          (1U<<16) // to understand the flag, see the description below.

// the systcik timer is a 24-bit down counter. it counts down from the loaded value to 0, be decremeting 1 on each clock cycle. 
// after it reaches 0, it sets the COUNTERFLAG bit to 1. 
// we can use this flag to detect when the timer has reached 0.
// after we read the flag, it automatically clears itself to 0.



void systick_delay_ms(int delay_ms) {
    // reload with the number of clocks per millisecond!!! 
    SysTick->LOAD = SYSTICK_LOAD_VAL;

    // clear the current value register!!!
    SysTick->VAL = 0;

    // enable the systick timer and select the processor clock source!!!
    SysTick->CTRL = CTRL_ENABLE | CTRL_CLKSOURCE;

    for (volatile int i=0; i<delay_ms; i++) {
        while ((SysTick->CTRL & CTRL_COUNTERFLAG) == 0){
            // do nothing!!!
        }

    }

    // disable the systick timer!!!
    SysTick->CTRL = 0;
}