#include <avr/io.h>
#include <util/delay.h>

void pin_output(volatile uint8_t *ddr, uint8_t pin) {
    *ddr |= (1 << pin);
}

void pin_input_pullup(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t pin) {
    *ddr &= ~(1 << pin);
    *port |= (1 << pin);
}

void pin_high(volatile uint8_t *port, uint8_t pin) {
    *port |= (1 << pin);
}

void pin_low(volatile uint8_t *port, uint8_t pin) {
    *port &= ~(1 << pin);
}

void pin_toggle(volatile uint8_t *port, uint8_t pin) {
    *port ^= (1 << pin);
}

int pin_read(volatile uint8_t *pin_reg, uint8_t pin) {
    return (*pin_reg & (1 << pin)) != 0;
}