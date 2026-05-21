#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

#define BUZZER_PIN PC2
#define BEEP_ON_TIME 80
#define BEEP_OFF_TIME 80

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_start_sound(uint8_t count);
void buzzer_update(void);

#endif