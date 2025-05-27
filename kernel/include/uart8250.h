#ifndef UART_8250_H
#define UART_8250_H
#include "types.h"


void uart8250_putc(char ch);
int uart8250_getc(void);

int uart8250_init(unsigned long base, uint32 in_freq, uint32 baudrate, uint32 reg_shift,
		  uint32 reg_width, uint32 reg_offset);
int uart8250_test();
int uart8250_change_base_addr(unsigned long base);

#endif