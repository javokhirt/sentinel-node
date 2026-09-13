
#ifndef UART_H_
#define UART_H_

void uart2_init(void);
static uint16_t compute_baudrate (uint32_t PeriphCLK, uint32_t BaudRate);
static void set_baudrate (USART_TypeDef *USARTx, uint32_t PeriphCLK, uint32_t BaudRate);
static void usart2_write(int ch);
ssize_t _write (int file, const char *ptr, ssize_t len);

#endif /* UART_H_ */