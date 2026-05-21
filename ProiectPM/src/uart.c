#include <avr/io.h>
#include "uart.h"
#include "const.h"

void uart_init(void) {
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)UBRR_VALUE;

    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit
}

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_print(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

int uart_is_available() {
    if (!(UCSR0A & (1 << RXC0))) {
        return 0;
    }

    return 1;
}

char uart_getc(void) {
    if (!uart_is_available()) {
        return 0;
    }
    return UDR0;
}