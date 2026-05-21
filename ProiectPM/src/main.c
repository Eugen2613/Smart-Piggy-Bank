#include "../include/lcd.h"
#include "../include/keypad.h"
#include "../include/timer.h"

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#define SENSOR_5BANI   PC5
#define SENSOR_10BANI  PC4
#define SENSOR_50BANI  PC3

#define BUZZER_PIN PC2

#define BEEP_ON_TIME   80
#define BEEP_OFF_TIME  80

void servo_init(void);
void servo_angle(uint8_t angle);

uint16_t total_bani = 0;

uint8_t last_5 = 1;
uint8_t last_10 = 1;
uint8_t last_50 = 1;

uint8_t beep_active = 0;
uint8_t beep_state = 0;
uint8_t beep_done = 0;
uint8_t beep_target = 0;
uint32_t beep_last_time = 0;

uint8_t trapa_deschisa = 0;

void buzzer_init(void) {
    DDRC |= (1 << BUZZER_PIN);
    PORTC &= ~(1 << BUZZER_PIN);
}

void buzzer_on(void) {
    PORTC |= (1 << BUZZER_PIN);
}

void buzzer_off(void) {
    PORTC &= ~(1 << BUZZER_PIN);
}

void buzzer_start_sound(uint8_t count) {
    beep_target = count;
    beep_done = 0;
    beep_active = 1;
    beep_state = 1;
    beep_last_time = timer2_millis_get();
    buzzer_on();
}

void buzzer_update(void) {
    if (!beep_active) return;

    uint32_t now = timer2_millis_get();

    if (beep_state == 1) {
        if (now - beep_last_time >= BEEP_ON_TIME) {
            buzzer_off();
            beep_state = 0;
            beep_done++;
            beep_last_time = now;
        }
    } else {
        if (beep_done >= beep_target) {
            beep_active = 0;
            buzzer_off();
            return;
        }

        if (now - beep_last_time >= BEEP_OFF_TIME) {
            buzzer_on();
            beep_state = 1;
            beep_last_time = now;
        }
    }
}

void sensors_init(void) {
    DDRC &= ~(1 << SENSOR_5BANI);
    DDRC &= ~(1 << SENSOR_10BANI);
    DDRC &= ~(1 << SENSOR_50BANI);

    PORTC |= (1 << SENSOR_5BANI);
    PORTC |= (1 << SENSOR_10BANI);
    PORTC |= (1 << SENSOR_50BANI);
}

void lcd_print_money(uint16_t bani) {
    char buffer[17];

    uint16_t lei = bani / 100;
    uint16_t rest = bani % 100;

    lcd_clear();

    lcd_set_cursor(0, 0);
    lcd_print("Suma:");

    lcd_set_cursor(0, 1);
    sprintf(buffer, "%u.%02u RON", lei, rest);
    lcd_print(buffer);
}

uint8_t wait_for_password(void) {
    const char password[] = "3456#";
    char input[6];
    uint8_t index = 0;

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Introdu parola:");
    lcd_set_cursor(0, 1);

    while (1) {
        char key = keypad_get_key_once();

        if (!key) continue;

        input[index++] = key;
        lcd_print("*");

        if (index == 5) {
            input[index] = '\0';

            if (strcmp(input, password) == 0) {
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Parola corecta");
                _delay_ms(1000);
                return 1;
            }

            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Parola gresita");
            _delay_ms(1000);

            index = 0;
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Introdu parola:");
            lcd_set_cursor(0, 1);
        }
    }
}

int main(void) {
    lcd_init();
    keypad_init();
    sensors_init();
    buzzer_init();

    timer2_init();

    servo_init();
    servo_angle(0);

    lcd_print_money(total_bani);

    while (1) {
        char key = keypad_get_key_once();

        if (key == '#') {
            if (trapa_deschisa == 0) {
                wait_for_password();

                servo_angle(180);
                trapa_deschisa = 1;

                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Trapa deschisa");
                _delay_ms(1000);

                lcd_print_money(total_bani);
            } else {
                servo_angle(0);
                trapa_deschisa = 0;

                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Trapa inchisa");
                _delay_ms(1000);

                lcd_print_money(total_bani);
            }
        }

        if (trapa_deschisa == 0) {
            uint8_t current_5 = (PINC & (1 << SENSOR_5BANI)) ? 1 : 0;
            uint8_t current_10 = (PINC & (1 << SENSOR_10BANI)) ? 1 : 0;
            uint8_t current_50 = (PINC & (1 << SENSOR_50BANI)) ? 1 : 0;

            if (last_5 == 1 && current_5 == 0) {
                total_bani += 5;
                lcd_print_money(total_bani);
                buzzer_start_sound(1);
            }

            if (last_10 == 1 && current_10 == 0) {
                total_bani += 10;
                lcd_print_money(total_bani);
                buzzer_start_sound(2);
            }

            if (last_50 == 1 && current_50 == 0) {
                total_bani += 50;
                lcd_print_money(total_bani);
                buzzer_start_sound(3);
            }

            last_5 = current_5;
            last_10 = current_10;
            last_50 = current_50;
        }

        buzzer_update();

        _delay_ms(1);
    }
}