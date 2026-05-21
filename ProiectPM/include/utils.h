#ifndef GPIO_H
#define GPIO_H

#include <avr/io.h>
#include <stdint.h>

void pin_output(volatile uint8_t *ddr, uint8_t pin);

void pin_input_pullup(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t pin);

void pin_high(volatile uint8_t *port, uint8_t pin);

void pin_low(volatile uint8_t *port, uint8_t pin);

void pin_toggle(volatile uint8_t *port, uint8_t pin);

int pin_read(volatile uint8_t *pin_reg, uint8_t pin);

#endif