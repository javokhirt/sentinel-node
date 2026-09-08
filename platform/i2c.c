#include "i2c.h"

#define GPIOBEN     (1U<<1)
#define I2C1EN      (1U<<21)
#define I2C1_CR1_EN  (1U<<0)
#define I2C1_CR1_START  (1U<<8)
#define I2C1_CR1_ACK    (1U<<10)
#define I2C1_CR1_STOP   (1U<<9)

#define I2C1_SR1_SB   (1U<<0)
#define I2C1_SR1_ADDR (1U<<1)
#define I2C1_SR1_TXE  (1U<<7)
#define I2C1_SR1_RXNE  (1U<<6)
#define I2C1_SR2_BUSY (1U<<1)

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

void I2C1_byteRead (char saddr, char maddr, char* data) {
    volatile int tmp;
// ------------------------------------- The START Condition
    // poll for I2C1 SR2 until it's not busy
    while (I2C1->SR2 & I2C1_SR2_BUSY) {
        // do nothing!
    }
    // generate the start condition
    I2C1->CR1 |= I2C1_CR1_START;

    //poll for I2C1 SR1 until the start flag is set
    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
        // do nothing!
    }
    // send the slave adress and WRITE
    I2C1->DR = saddr << 1;

    // poll for I2C1 SR1 until the address flag is set
    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
        // do nothing!
    }
    // clear the address flag by reading SR2
    tmp = I2C1->SR2

    // send memory address
    I2C1->DR = maddr;

    // poll for I2C1 SR1 until the TXE flag is set
    while (!(I2C1->SR1 & I2C1_SR1_TXE)) {
        // do nothing!
    }

    // ----------------------- The RESTART Condition -----------------------
    // generate the restart condition
    I2C1->CR1 |= I2C1_CR1_START;

    //poll for I2C1 SR1 until the start flag is set
    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
        // do nothing!
    }
    // send the slave adress and start reading
    I2C1->DR = (saddr << 1) | 1;

    // poll for I2C1 SR1 until the address flag is set
    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
        // do nothing!
    }
    // disable the acknowledge
    I2C1->CR1 &=~ I2C1_CR1_ACK;

    // clear the address flag by reading SR2
    tmp = I2C1->SR2

    //set the stop condition
    I2C1->CR1 |= I2C1_CR1_STOP;

    // wait until the DR register is not empty
    while (!(I2C1_SR1_RXNE & I2C1_SR1_RXNE)) {
        // do nothing!
    }
    // read and assign the data to the variable
    *data++ = I2C1->DR;
}



void I2C1_burstRead (char saddr, char maddr, int numBytes, char* data) {

    volatile int tmp;
    
    // poll for I2C SR2 until it's not busy
    while (!(I2C1->SR2 & I2C1_SR2_BUSY)) {
        // do nothing!
    }
    // set the start condition
    I2C1->CR1 |= I2C1_CR1_START;

    // poll until the (SB) flag is set
    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
        // do nothing!
    }
    // transmit the slave adress and write
    I2C1->DR = (saddr << 1);

    // poll until the address flag is set
    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
        // do nothing!
    }
    // clear the address flag
    tmp = I2C1->SR2;

    // transmit the maddr
    I2C1->DR = maddr;
    
    // poll the SR1 until the TXE flag is set
    while (!(I2C1->SR1 & I2C1_SR1_TXE)) {
        // do nothing!
    }
    //------------------------------------ The RESTART Condition ----------------------------------------
    // generate the restart condition
    I2C1->CR1 |= I2C1_CR1_START;

    // pol until the SR1 SB flag is set
    while (I2C1->SR1 & I2C1_SR1_SB) {
        // do nothing!
    }
    // transmit the slave adress and read
    I2C1->DR = (saddr<<1) | 1;

    // poll until the adress flag is set
    while (!(I2C1->SR1 & I2C1_SR1_ADDR)){
        // do nothing!
    }
    // clear the adress flag
    tmp = I2C1->SR2;

    // enable the ACK bit
    I2C1->CR1 |= I2C1_CR1_ACK;

    while (n > 0U) {
        // if one byte
        if (n == 1) {
            // disable the ACK
            I2C1->CR1 &=~ I2C1_CR1_ACK;
            // generare STOP
            I2C1->CR1 |= I2C1_CR1_STOP;
            // wait for RNXE flag is set
            while (!(I2C1->SR1 & I2C1_SR1_RXNE)){
                // do nothing!
            }
            // read the data
            *data++ = I2C1->DR;
            break;
        }
        else {
            // wait until the RXNE flag is set
            while (!(I2C1->SR1 & I2C1_SR1_RXNE)){
                // do nothing!
            }
            // read the data
            *data++ = I2C1->DR;
            n--;
        }
    }
    




}