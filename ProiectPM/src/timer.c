#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

static volatile uint32_t millis_counter = 0;

ISR(TIMER1_COMPA_vect) {
    millis_counter++;
}

void timer2_init(void) {
    // Timer1 CTC mode, 1ms interrupt
    TCCR1A = 0;
    TCCR1B = 0;

    // 16MHz / 64 = 250kHz
    // 250 ticks = 1ms => OCR1A = 249
    OCR1A = 249;

    TCCR1B |= (1 << WGM12);              // CTC mode
    TCCR1B |= (1 << CS11) | (1 << CS10); // prescaler 64

    TIMSK1 |= (1 << OCIE1A);             // enable compare interrupt

    sei();
}

uint32_t timer2_millis_get(void) {
    uint32_t value;

    cli();
    value = millis_counter;
    sei();

    return value;
}