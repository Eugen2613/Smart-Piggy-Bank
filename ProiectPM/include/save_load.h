#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include <stdint.h>

#define PASSWORD_LEN 4

void total_load(uint16_t *total_bani);
void total_save(uint16_t total_bani);

void password_load(char *password);
void password_save(char *password);

#endif