#include <avr/io.h>
#include <util/delay.h>

void servo_init(void) {
    // PD3 = OC2B output
    DDRD |= (1 << PD3);

    // Fast PWM
    TCCR2A =
        (1 << COM2B1) |
        (1 << WGM21) |
        (1 << WGM20);

    // Prescaler 1024
    TCCR2B =
        (1 << CS22) |
        (1 << CS21) |
        (1 << CS20);
}

void servo_angle(uint8_t angle) {
    // map 0-180 -> aproximativ 8-38
    OCR2B = 8 + ((uint16_t)angle * 30) / 180;
}