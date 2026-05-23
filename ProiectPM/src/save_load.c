#include "../include/save_load.h"

#include <avr/eeprom.h>
#include <string.h>

uint8_t EEMEM ee_password[PASSWORD_LEN + 1];
uint8_t EEMEM ee_password_valid;

uint16_t EEMEM ee_total_bani;
uint8_t EEMEM ee_total_valid;

void total_load(uint16_t *total_bani) {
    if (eeprom_read_byte(&ee_total_valid) == 0xA5) {
        *total_bani = eeprom_read_word(&ee_total_bani);
    } else {
        *total_bani = 0;
        eeprom_update_word(&ee_total_bani, *total_bani);
        eeprom_update_byte(&ee_total_valid, 0xA5);
    }
}

void total_save(uint16_t total_bani) {
    eeprom_update_word(&ee_total_bani, total_bani);
    eeprom_update_byte(&ee_total_valid, 0xA5);
}

void password_load(char *password) {
    uint8_t valid = eeprom_read_byte(&ee_password_valid);

    if (valid == 0xA5) {
        eeprom_read_block(password, ee_password, PASSWORD_LEN + 1);
        password[PASSWORD_LEN] = '\0';
    } else {
        strcpy(password, "3456");
        eeprom_update_block(password, ee_password, PASSWORD_LEN + 1);
        eeprom_update_byte(&ee_password_valid, 0xA5);
    }
}

void password_save(char *password) {
    eeprom_update_block(password, ee_password, PASSWORD_LEN + 1);
    eeprom_update_byte(&ee_password_valid, 0xA5);
}