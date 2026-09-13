#include "stm32f4xx.h"
#include <stdint.h>
#include <sys/types.h> 
#include "uart.h"


#define GPIOAEN       (1U<<0)
#define USART2EN      (1U<<17)
#define USART2_TE     (1U<<3)
#define USART2_UE     (1U<<13)
#define SR_TXE        (1U<<7)

#define SYS_FREQ      (16000000)
#define APB1_CLK      (SYS_FREQ) 
#define BAUD_RATE     (115200)



ssize_t _write(int file, const char *ptr, ssize_t len) {
    (void)file;
    for (ssize_t i = 0; i < len; i++) {
        usart2_write((unsigned char)ptr[i]);
    }
    return len;
}
void uart2_init(void) {
    // 1. Enable clock access to GPIOA
    RCC->AHB1ENR |= GPIOAEN;
    // 2. Set PA2 to alternate function mode
    GPIOA->MODER |= (1U<<5);
    GPIOA->MODER &=~ (1U<<4);
    // 3. Set PA2 alternate function type to UART_TX (AF07)
    GPIOA->AFR[0] |= (1U<<8);
    GPIOA->AFR[0] |= (1U<<9);
    GPIOA->AFR[0] |= (1U<<10);
    GPIOA->AFR[0] &=~ (1U<<11);



    RCC->APB1ENR |= USART2EN;

    set_baudrate (USART2, APB1_CLK, BAUD_RATE);

    USART2->CR1 = USART2_TE; 

    USART2->CR1 |= USART2_UE; 
}

static void usart2_write(int ch) {
    while (!(USART2->SR & SR_TXE))  {
        // poll
    }
    USART2->DR = (ch & 0xFF);
}

static void set_baudrate (USART_TypeDef *USARTx, uint32_t PeriphCLK, uint32_t BaudRate) {
    USARTx->BRR = compute_baudrate(PeriphCLK, BaudRate);
}

static uint16_t compute_baudrate (uint32_t PeriphCLK, uint32_t BaudRate) {
    return((PeriphCLK + (BaudRate/2U)) / BaudRate);
} 