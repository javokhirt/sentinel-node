#include "i2c.h"
#include "stm32f4xx.h"
#include <stdint.h>

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
#define I2C1_SR1_BTF   (1U<<2)
#define I2C1_SR2_BUSY (1U<<1)

void i2c1_init(void) {

    RCC->AHB1ENR |= GPIOBEN;
    /* PB8 = SCL, PB9 = SDA, AF4 */
    GPIOB->MODER &=~ (1U<<16);
    GPIOB->MODER |= (1U<<17);
    GPIOB->MODER &=~ (1U<<18);
    GPIOB->MODER |= (1U<<19);
    GPIOB->AFR[1] |= (1U<<2);
    GPIOB->AFR[1] |= (1U<<6);

    GPIOB->OTYPER |= (1U<<8) | (1U<<9);
    GPIOB->PUPDR |= (1U<<16) | (1U<<18);

    RCC->APB1ENR |= I2C1EN;

    I2C1->CR1 |= (1U<<15);
    I2C1->CR1 &=~ (1U<<15);

    /* CR2.FREQ = APB1 clock in MHz (16 MHz HSI) */
    I2C1->CR2 |= (16U<<0);

    /* CCR = 80 → 100 kHz standard mode at 16 MHz: Thigh = Tlow = 80 * T_PCLK1 */
    I2C1->CCR = (80U<<0);

    /* TRISE = (max SCL rise time / T_PCLK1) + 1 = 1000 ns / 62.5 ns + 1 = 17 */
    I2C1->TRISE = (17U<<0);

    I2C1->CR1 |= I2C1_CR1_EN;

}
// only accepts n>=3
void i2c1_read (uint8_t saddr, uint16_t n, uint8_t* data) {

    volatile int tmp;

    while (I2C1->SR2 & I2C1_SR2_BUSY) {
    }
    I2C1->CR1 |= I2C1_CR1_START;

    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
    }
    I2C1->DR = (saddr << 1) | 1;

    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
    }
    /* Hardware ACK after each received byte until the last */
    I2C1->CR1 |= I2C1_CR1_ACK;
    
    tmp = I2C1->SR2;


    while (n > 3U) {
        while (!(I2C1->SR1 & I2C1_SR1_RXNE)){
        }
        *data++ = I2C1->DR;
        n--;
    }
    
    while (!(I2C1->SR1 & I2C1_SR1_BTF)){}
    I2C1->CR1 &=~ I2C1_CR1_ACK;
    *data++ = I2C1->DR;
    I2C1->CR1 |= I2C1_CR1_STOP;
    *data++ = I2C1->DR; // read byte before the last one
    while (!(I2C1->SR1 & I2C1_SR1_RXNE)){}
    *data = I2C1->DR; // read the last byte

}

void i2c1_write (uint8_t saddr, uint16_t n, const uint8_t* data) {
    volatile int tmp;

    while (I2C1->SR2 & I2C1_SR2_BUSY){
    }
    I2C1->CR1 |= I2C1_CR1_START;

    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
    }
    I2C1->DR = (saddr<<1);

    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
    }
    tmp = I2C1->SR2;

    for (int i = 0; i < n; i++) {
        while (!(I2C1->SR1 & I2C1_SR1_TXE)) {
        }
        I2C1->DR = *data++;
    }
    /* BTF: last byte has left DR and is on the bus — then STOP */
    while (!(I2C1->SR1 & I2C1_SR1_BTF)){
    }

    I2C1->CR1 |= I2C1_CR1_STOP;

}
