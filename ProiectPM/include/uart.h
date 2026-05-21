#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_putc(char c);
void uart_print(const char *s);
char uart_getc(void);

#endif