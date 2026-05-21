#include "../include/utils.h"
#include "../include/timer.h"

#define BUZZER_PIN PC2
#define BEEP_ON_TIME   80
#define BEEP_OFF_TIME  80

uint8_t beep_active = 0;
uint8_t beep_state = 0;
uint8_t beep_done = 0;
uint8_t beep_target = 0;
uint32_t beep_last_time = 0;

void buzzer_init(void) {
    pin_output(&DDRC, BUZZER_PIN);
    pin_low(&PORTC, BUZZER_PIN);
}

void buzzer_on(void) {
    pin_high(&PORTC, BUZZER_PIN);
}

void buzzer_off(void) {
    pin_low(&PORTC, BUZZER_PIN);
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
    if (!beep_active) {
        return;
    }

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