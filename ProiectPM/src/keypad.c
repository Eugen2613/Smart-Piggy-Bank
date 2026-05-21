#include "../include/uart.h"
#include "../include/utils.h"
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define ROW1 PB2
#define ROW2 PB3
#define ROW3 PB4
#define ROW4 PB5

#define COL1 PD0
#define COL2 PD1
#define COL3 PD2

#define ROWS_COUNT 4
#define COLS_COUNT 3

const uint8_t rows[ROWS_COUNT] = {ROW1, ROW2, ROW3, ROW4};
const uint8_t cols[COLS_COUNT] = {COL1, COL2, COL3};
const char keys[4][3] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};

void keypad_init() {
    for (int i = 0; i < ROWS_COUNT; i++) {
        // outputs HIGH
        pin_output(&DDRB, rows[i]);
        pin_high(&PORTB, rows[i]);
    }

    for (int i = 0; i < COLS_COUNT; i++) {
        // inputs with pull-up
        pin_input_pullup(&DDRD, &PORTD, cols[i]);
    }
}

char detect_key() {
    uint8_t found = 0;
    char key = 0;

    // Setting all rows to high
    for (int i = 0; i < ROWS_COUNT; i++) {
        pin_high(&PORTB, rows[i]);
    }

    for (int i = 0; i < ROWS_COUNT; i++) {
        pin_low(&PORTB, rows[i]);
        _delay_us(5);
        for (int j = 0; j < COLS_COUNT; j++) {
            if (!(PIND & (1 << cols[j]))) {
                key = keys[i][j];
                found = 1;
                pin_high(&PORTB, rows[i]);
                break;
            }
        }

        if (found) {
            break;
        }

        pin_high(&PORTB, rows[i]);
    }

    return key;
}

char keypad_get_key_once(void) {
    static char last_key = 0;
    char key = detect_key();

    if (key != 0 && last_key == 0) {
        _delay_ms(30);
        if (detect_key() == key) {
            last_key = key;
            return key;
        }
    }

    if (key == 0) {
        _delay_ms(30);
        if (detect_key() == 0) {
            last_key = 0;
        }
    }

    return 0;
}