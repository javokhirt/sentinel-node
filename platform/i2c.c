#include "i2c.h"


#define GPIOBEN     (1U<<1)
#define I2C1EN      (1U<<21)
#define I2C1_CR1_EN  (1U<<0);


void I2C1_Init(void) {
    
    // enable clock access to GPIOB 
    RCC->AHB1ENR |= GPIOBEN;
    // set PB8 and PB9 mode to alternate function AF04
    GPIOB->MODER &=~ (1U<<16);
    GPIOB->MODER |= (1U<<17);
    GPIOB->MODER &=~ (1U<<18);
    GPIOB->MODER |= (1U<<19);
    GPIOB->AFR[1] |= (1U<<2);
    GPIOB->AFR[1] |= (1U<<6);

    // set PB8 and PB9 output type to open drain
    GPIOB->OTYPER |= (1U<<8) | (1U<<9);
    // enable pull-up resistors for PB8 and PB9
    GPIOB->PUPDR |= (1U<<16) | (1U<<18);

    // enable clock access to I2C1
    RCC->APB1ENR |= I2C1EN;

    // enter the reset mode
    I2C1->CR1 |= (1U<<15);
    
    // exit the reset mode
    I2C1->CR1 &=~ (1U<<15);

    // set the I2C1 clock frequency to 16mhz 
    I2C1->CR2 |= (16U<<0);

    // set the I2C1 CCR to 80 to achieve 100khz clock frequency for (SM)
    I2C1->CCR = (80U<<0);

    // set the I2C1 TRISE regsiter to value 17 - max possible rise time for (SM)
    // we should divide the max rise time by the 1 clock tick(SYSCLK)
    I2C1->TRISE = (17U<<0);

    // enable the I2C1
    I2C1->CR1 |= I2C1_CR1_EN;

}