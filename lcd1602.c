// ==========================================================================
// 1602 LCD 4비트 모드 제어 드라이버 구현부 (lcd1602.c)
// ==========================================================================

#include "lcd1602.h"

// 4비트 데이터를 포트에 싣고 가동 펄스(E)를 발생시키는 내부 함수
static void lcd_send_nibble(uint8_t nibble)
{
	// 상위 4비트(PC4~PC7) 영역만 데이터 교체
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (nibble & 0xF0);
	
	// Enable 핀 하이/로우 트리거로 LCD에 데이터 쓰기 완료
	LCD_DATA_PORT |= (1 << LCD_E);
	_delay_us(1);
	LCD_DATA_PORT &= ~(1 << LCD_E);
	_delay_us(1);
}

// 8비트 데이터를 상위/하위 4비트씩 2회 분할 전송 (mode: 0=명령어, 1=일반 데이터)
static void lcd_send_byte(uint8_t value, uint8_t mode)
{
	if (mode == 0) LCD_DATA_PORT &= ~(1 << LCD_RS); // RS = 0 : 명령어 모드 지정
	else           LCD_DATA_PORT |= (1 << LCD_RS);  // RS = 1 : 데이터 표시 모드 지정
	
	LCD_DATA_PORT &= ~(1 << LCD_RW); // R/W = 0 : 쓰기(Write) 모드로 고정
	
	lcd_send_nibble(value & 0xF0);        // 1단계: 상위 4비트 먼저 전송
	lcd_send_nibble((value << 4) & 0xF0); // 2단계: 하위 4비트를 끌어올려 전송
	
	_delay_us(50); // 명령어 처리 최소 마진 대기
}

// 외부 노출용 명령어 송신 인터페이스
void lcd_command(uint8_t cmd)
{
	lcd_send_byte(cmd, 0);
}

// 외부 노출용 한 글자(Data) 표시 인터페이스
void lcd_data(uint8_t data)
{
	lcd_send_byte(data, 1);
}

// 데이터시트 규격에 맞춘 4비트 환경 전용 초기화 시퀀스
void lcd_init(void)
{
	// 1. 데이터 및 제어 핀 포트 방향 출력 설정
	LCD_DATA_DDR |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E) | 0xF0;
	LCD_DATA_PORT &= ~((1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E) | 0xF0);
	
	_delay_ms(20); // Power On 이후 초기 전압 상승 대기
	
	// 2. 소프트웨어 리셋: 8비트 모드 명령(0x30)을 강제로 3회 주입해 내부 칩 리셋
	lcd_send_nibble(0x30);
	_delay_ms(5);
	lcd_send_nibble(0x30);
	_delay_us(150);
	lcd_send_nibble(0x30);
	_delay_us(150);
	
	// 3. 4비트 모드 가동 명령(0x20) 전송
	lcd_send_nibble(0x20);
	_delay_ms(2);
	
	// 4. 레지스터 환경 설정 명령 순차 송신
	lcd_command(LCD_CMD_FUNCTION_SET); // 4비트 버스, 2줄 출력 설정
	lcd_command(LCD_CMD_DISPLAY_ON);  // 디스플레이 화면 활성화
	lcd_clear();                      // 화면 청소
	lcd_command(LCD_CMD_ENTRY_MODE);   // 주소 자동 증가 방향 설정
}

// 문자열을 연속으로 화면에 출력하는 함수
void lcd_string(const char *str)
{
	while (*str)
	{
		lcd_data((uint8_t)(*str++));
	}
}

// 화면 전체를 지우는 함수 (Clear 명령어는 내부 처리 시간이 길어 2ms 대기 필수)
void lcd_clear(void)
{
	lcd_command(LCD_CMD_CLEAR);
	_delay_ms(2);
}

// 행/열 좌표로 커서를 이동시키는 함수 (row: 0=첫째줄, 1=둘째줄 / col: 0~15)
void lcd_goto_xy(uint8_t row, uint8_t col)
{
	uint8_t address = 0;
	
	if (row == 0) address = 0x00 + col; // 1번째 줄 시작 주소 기반 연산
	else          address = 0x40 + col; // 2번째 줄 시작 주소 기반 연산
	
	lcd_command(0x80 | address); // DDRAM 주소 지정 커맨드 비트(0x80)와 결합하여 전송
}

// 하드웨어 커서 모드 제어 함수 (visible: 밑줄 생성, blink: 네모 박스 깜빡임)
void lcd_set_cursor_mode(uint8_t visible, uint8_t blink)
{
	uint8_t command = LCD_CMD_DISPLAY_ON; // 기본 디스플레이 On 상태 베이스
	
	if (visible) command |= 0x02; // 커서 밑줄 활성화
	if (blink)   command |= 0x01; // 커서 블링크 네모 박스 활성화
	
	lcd_command(command);
}