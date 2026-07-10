/**
* @file    lcd1602.h
* @brief   1602 Character LCD 4비트 모드 제어 드라이버 헤더
* @author  kccistc
*/

#ifndef LCD1602_H_
#define LCD1602_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define LCD_DATA_DDR      DDRC
#define LCD_DATA_PORT     PORTC

#define LCD_RS            0   // PORTC0
#define LCD_RW            1   // PORTC1
#define LCD_E             2   // PORTC2

#define LCD_CMD_CLEAR           0x01   // 화면 지우기
#define LCD_CMD_RETURN_HOME     0x02   // 커서 홈 위치 복귀
#define LCD_CMD_ENTRY_MODE      0x06   // 커서 우측 이동, 화면 시프트 안함
#define LCD_CMD_DISPLAY_ON      0x0C   // 디스플레이 On, 커서 Off, 깜빡임 Off
#define LCD_CMD_FUNCTION_SET    0x28   // 4비트 모드, 2줄 출력, 5x8 도트 폰트

void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_string(const char *str);
void lcd_clear(void);
void lcd_goto_xy(uint8_t row, uint8_t col);
void lcd_set_cursor_mode(uint8_t visible, uint8_t blink);

#endif /* LCD1602_H_ */