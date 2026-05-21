#ifndef MOTOR_H
#define MOTOR_H
#include <stdint.h>

void servo_init(void);
void servo_angle(uint8_t angle);

#endif