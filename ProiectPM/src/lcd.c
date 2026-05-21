#include <avr/io.h>
#include <util/delay.h>

#define LCD_RS_PORT PORTB
#define LCD_RS_DDR  DDRB
#define LCD_RS_PIN  PB0

#define LCD_EN_PORT PORTB
#define LCD_EN_DDR  DDRB
#define LCD_EN_PIN  PB1

#define LCD_DATA_PORT PORTD
#define LCD_DATA_DDR  DDRD

#define LCD_D4 PD4
#define LCD_D5 PD5
#define LCD_D6 PD6
#define LCD_D7 PD7

void lcd_enable_pulse(void) {
    LCD_EN_PORT |= (1 << LCD_EN_PIN);
    _delay_us(1);
    LCD_EN_PORT &= ~(1 << LCD_EN_PIN);
    _delay_us(100);
}

void lcd_send_nibble(uint8_t nibble) {
    LCD_DATA_PORT &= ~((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));

    if (nibble & 0x01) LCD_DATA_PORT |= (1 << LCD_D4);
    if (nibble & 0x02) LCD_DATA_PORT |= (1 << LCD_D5);
    if (nibble & 0x04) LCD_DATA_PORT |= (1 << LCD_D6);
    if (nibble & 0x08) LCD_DATA_PORT |= (1 << LCD_D7);

    lcd_enable_pulse();
}

void lcd_command(uint8_t cmd) {
    LCD_RS_PORT &= ~(1 << LCD_RS_PIN);

    lcd_send_nibble(cmd >> 4);
    lcd_send_nibble(cmd & 0x0F);

    _delay_ms(2);
}

void lcd_data(uint8_t data) {
    LCD_RS_PORT |= (1 << LCD_RS_PIN);

    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);

    _delay_ms(2);
}

void lcd_clear(void) {
    lcd_command(0x01);
    _delay_ms(2);
}

void lcd_init(void) {
    LCD_RS_DDR |= (1 << LCD_RS_PIN);
    LCD_EN_DDR |= (1 << LCD_EN_PIN);

    LCD_DATA_DDR |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

    _delay_ms(50);

    LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
    LCD_EN_PORT &= ~(1 << LCD_EN_PIN);

    lcd_send_nibble(0x03);
    _delay_ms(5);

    lcd_send_nibble(0x03);
    _delay_us(150);

    lcd_send_nibble(0x03);
    _delay_us(150);

    lcd_send_nibble(0x02);

    lcd_command(0x28); // 4-bit, 2 lines, 5x8 font
    lcd_command(0x0C); // display ON, cursor OFF
    lcd_command(0x06); // cursor moves right
    lcd_command(0x01); // clear display
    _delay_ms(2);
}

void lcd_print(const char *text) {
    while (*text) {
        lcd_data(*text++);
    }
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t address = (row == 0) ? 0x00 : 0x40;
    address += col;
    lcd_command(0x80 | address);
}
