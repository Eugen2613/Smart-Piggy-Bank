#include "../include/lcd.h"
#include "../include/keypad.h"
#include "../include/timer.h"
#include "../include/motor.h"
#include "../include/utils.h"
#include "../include/buzzer.h"
#include "../include/save_load.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>

#define SENSOR_5BANI   PC5
#define SENSOR_10BANI  PC4
#define SENSOR_50BANI  PC3

#define COIN_COOLDOWN  400
#define PASSWORD_LEN 4
#define STAR_HOLD_TIME 3000

char detect_key(void);
void handle_coin_flags(void);

uint16_t total_bani = 0;

char password[PASSWORD_LEN + 1];

uint8_t trapa_deschisa = 0;

uint32_t last_detect_5 = 0;
uint32_t last_detect_10 = 0;
uint32_t last_detect_50 = 0;

volatile uint8_t coin5_flag = 0;
volatile uint8_t coin10_flag = 0;
volatile uint8_t coin50_flag = 0;
volatile uint8_t last_pin_c = 0;

void sensors_init(void) {
    pin_input_pullup(&DDRC, &PORTC, SENSOR_5BANI);
    pin_input_pullup(&DDRC, &PORTC, SENSOR_10BANI);
    pin_input_pullup(&DDRC, &PORTC, SENSOR_50BANI);
}

void coin_interrupts_init(void) {
    last_pin_c = PINC;

    PCICR |= (1 << PCIE1);

    PCMSK1 |= (1 << PCINT13);
    PCMSK1 |= (1 << PCINT12);
    PCMSK1 |= (1 << PCINT11);

    sei();
}

ISR(PCINT1_vect) {
    uint8_t current = PINC;
    uint8_t changed = current ^ last_pin_c;

    if ((changed & (1 << SENSOR_5BANI)) && !(current & (1 << SENSOR_5BANI))) {
        coin5_flag = 1;
    }

    if ((changed & (1 << SENSOR_10BANI)) && !(current & (1 << SENSOR_10BANI))) {
        coin10_flag = 1;
    }

    if ((changed & (1 << SENSOR_50BANI)) && !(current & (1 << SENSOR_50BANI))) {
        coin50_flag = 1;
    }

    last_pin_c = current;
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

uint8_t read_password_input(const char *message, char *out) {
    uint8_t index = 0;

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(message);
    lcd_set_cursor(0, 1);

    while (1) {
        char key = keypad_get_key_once();

        buzzer_update();
        handle_coin_flags();

        if (!key) {
            continue;
        }

        if (key == '#') {
            lcd_print_money(total_bani);
            return 0;
        }

        if (key >= '0' && key <= '9') {
            out[index++] = key;
            lcd_print("*");

            if (index == PASSWORD_LEN) {
                out[PASSWORD_LEN] = '\0';
                return 1;
            }
        }
    }
}

uint8_t read_money_input(const char *message, uint16_t *value) {
    char buffer[6];
    uint8_t index = 0;

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(message);
    lcd_set_cursor(0, 1);

    while (1) {
        char key = keypad_get_key_once();

        buzzer_update();

        if (!key) {
            continue;
        }

        if (key == '#') {
            lcd_print_money(total_bani);
            return 0;
        }

        if (key == '*') {
            if (index == 0) {
                continue;
            }

            buffer[index] = '\0';
            *value = (uint16_t)atoi(buffer);
            return 1;
        }

        if (key >= '0' && key <= '9' && index < 5) {
            buffer[index++] = key;
            lcd_data(key);
        }
    }
}

void withdraw_money(void) {
    uint16_t scos = 0;

    if (!trapa_deschisa) {
        return;
    }

    if (!read_money_input("Suma scoasa:", &scos)) {
        return;
    }

    if (scos > total_bani) {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Suma prea mare");
        _delay_ms(1000);
        lcd_print_money(total_bani);
        return;
    }

    total_bani -= scos;
    total_save(total_bani);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Suma modificata");
    _delay_ms(1000);

    lcd_print_money(total_bani);
}

uint8_t wait_for_password(void) {
    char input[PASSWORD_LEN + 1];

    while (1) {
        if (!read_password_input("Introdu parola:", input)) {
            return 0;
        }

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
    }
}

void change_password(void) {
    char old_pass[PASSWORD_LEN + 1];
    char new_pass[PASSWORD_LEN + 1];

    if (!read_password_input("Parola veche:", old_pass)) {
        return;
    }

    if (strcmp(old_pass, password) != 0) {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Parola gresita");
        _delay_ms(1000);
        lcd_print_money(total_bani);
        return;
    }

    if (!read_password_input("Parola noua:", new_pass)) {
        return;
    }

    strcpy(password, new_pass);
    password_save(password);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Parola schimbata");
    _delay_ms(1000);

    lcd_print_money(total_bani);
}

void check_star_hold(void) {
    static uint8_t star_pressed = 0;
    static uint8_t already_triggered = 0;
    static uint32_t start_time = 0;

    char key = detect_key();
    uint32_t now = timer2_millis_get();

    if (trapa_deschisa) {
        star_pressed = 0;
        already_triggered = 0;
        return;
    }

    if (key == '*') {
        if (!star_pressed) {
            star_pressed = 1;
            already_triggered = 0;
            start_time = now;
        }

        if (!already_triggered && now - start_time >= STAR_HOLD_TIME) {
            already_triggered = 1;
            change_password();
        }
    } else {
        star_pressed = 0;
        already_triggered = 0;
    }
}

void handle_coin_flags(void) {
    uint32_t now = timer2_millis_get();

    if (coin5_flag) {
        coin5_flag = 0;

        if (trapa_deschisa == 0 && now - last_detect_5 >= COIN_COOLDOWN) {
            last_detect_5 = now;
            total_bani += 5;
            total_save(total_bani);
            lcd_print_money(total_bani);
            buzzer_start_sound(1);
        }
    }

    if (coin10_flag) {
        coin10_flag = 0;

        if (trapa_deschisa == 0 && now - last_detect_10 >= COIN_COOLDOWN) {
            last_detect_10 = now;
            total_bani += 10;
            total_save(total_bani);
            lcd_print_money(total_bani);
            buzzer_start_sound(2);
        }
    }

    if (coin50_flag) {
        coin50_flag = 0;

        if (trapa_deschisa == 0 && now - last_detect_50 >= COIN_COOLDOWN) {
            last_detect_50 = now;
            total_bani += 50;
            total_save(total_bani);
            lcd_print_money(total_bani);
            buzzer_start_sound(3);
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

    coin_interrupts_init();
    password_load(password);
    total_load(&total_bani);

    lcd_print_money(total_bani);

    while (1) {
        char key = keypad_get_key_once();

        if (key == '#') {
            if (trapa_deschisa == 0) {
                if (wait_for_password()) {
                    servo_angle(180);
                    trapa_deschisa = 1;

                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_print("Trapa deschisa");
                    _delay_ms(1000);

                    lcd_print_money(total_bani);
                }
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

        if (key == '*' && trapa_deschisa) {
            withdraw_money();
        }

        check_star_hold();
        handle_coin_flags();
        buzzer_update();

        _delay_ms(1);
    }
}