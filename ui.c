// ==========================================================================
// 디스플레이 렌더링 및 모달 버튼 이벤트 통합 관리 구현부 (ui.c)
// ==========================================================================
#include "ui.h"
#include "lcd1602.h"
#include "cal.h"
#include "ds1307.h"
#include <stdio.h>

volatile uint8_t		is_edit_mode			= 0;
volatile edit_field_t	edit_field				= EDIT_FIELD_YEAR;
volatile uint8_t		should_render_display	= 1;
date_time_t				date_time				= { 0, 0, 12, 1, 1, 7, 2026 };
volatile display_mode_t	display_mode			= MODE_CLOCK;

static void handle_button_0(void);
static void handle_button_1(void);
static void handle_button_2(void);
static void handle_button_3(void);
static void handle_button_4(void);

static void (*button_handlers[])(void) = {
	handle_button_0,
	handle_button_1,
	handle_button_2,
	handle_button_3,
	handle_button_4
};

void ui_init(void)
{
	lcd_init();
}

// 버튼 이벤트 처리
void ui_handle_button(uint8_t button_id)
{
	button_handlers[button_id]();
}

// LCD 화면 갱신
void ui_update(void)
{
	char ui_buffer[17];
	char cal_line1[17], cal_line2[17];
	
	if (display_mode == MODE_CLOCK)
	{
		// [시계 모드] 디스플레이 출력
		lcd_goto_xy(0, 0);
		sprintf(ui_buffer, "DATE: %04d-%02d-%02d", date_time.year, date_time.month, date_time.day);
		lcd_string(ui_buffer);

		lcd_goto_xy(1, 0);
		sprintf(ui_buffer, "TIME: %02d:%02d:%02d", date_time.hour, date_time.minute, date_time.second);
		lcd_string(ui_buffer);

		// 시계 수정 상태에 따른 하드웨어 타깃 커서 위치 지정
		if (is_edit_mode)
		{
			lcd_set_cursor_mode(1, 1); // 깜빡이는 네모 커서 활성화
			switch (edit_field)
			{
				case EDIT_FIELD_YEAR:
				lcd_goto_xy(0, 6);
				break;
				
				case EDIT_FIELD_MONTH:
				lcd_goto_xy(0, 11);
				break;
				
				case EDIT_FIELD_DAY:
				lcd_goto_xy(0, 14);
				break;
				
				case EDIT_FIELD_HOUR:
				lcd_goto_xy(1, 6);
				break;
				
				case EDIT_FIELD_MINUTE:
				lcd_goto_xy(1, 9);
				break;
				
				case EDIT_FIELD_SECOND:
				lcd_goto_xy(1, 12);
				break;
				
				default:
				break;
			}
		}
		else
		{
			lcd_set_cursor_mode(0, 0); // 일반 모드일 때는 커서 숨김
		}
	}
	else
	{
		// [계산기 모드] 디스플레이 출력
		lcd_set_cursor_mode(0, 0);
		cal_get_display_strings(cal_line1, cal_line2); // 가공 완료된 문자열 배열 호출
		
		lcd_goto_xy(0, 0);
		lcd_string(cal_line1); // 첫 번째 줄: 수식 표출
		
		lcd_goto_xy(1, 0);
		lcd_string(cal_line2); // 두 번째 줄: 계산 결과 표출
	}
}

// [사이드 버튼 0] 시계 모드: 값 감소 (-) / 계산기 모드: 올 클리어 (C)
static void handle_button_0(void)
{
	if (display_mode == MODE_CLOCK)
	{
		if (is_edit_mode == 0) return; // 수정 모드가 아니면 차단
		switch (edit_field)
		{
			case EDIT_FIELD_YEAR:
			date_time.year--;
			if (date_time.year < 2000) date_time.year = 2099;
			break;
			
			case EDIT_FIELD_MONTH:
			date_time.month--;
			if (date_time.month < 1) date_time.month = 12;
			break;
			
			case EDIT_FIELD_DAY:
			date_time.day--;
			if (date_time.day < 1) date_time.day = 31;
			break;
			
			case EDIT_FIELD_HOUR:
			if (date_time.hour == 0) date_time.hour = 23;
			else date_time.hour--;
			break;
			
			case EDIT_FIELD_MINUTE:
			if (date_time.minute == 0) date_time.minute = 59;
			else date_time.minute--;
			break;
			
			case EDIT_FIELD_SECOND:
			if (date_time.second == 0) date_time.second = 59;
			else date_time.second--;
			break;
			
			default:
			break;
		}
	}
	else
	{
		cal_reset(); // 계산기 수식 초기화 및 화면 0 정렬 지시
	}
	
	should_render_display = 1;
}

// [사이드 버튼 1] 시계 모드: 값 증가 (+) / 계산기 모드: 한 글자 백스페이스 (◀)
static void handle_button_1(void)
{
	if (display_mode == MODE_CLOCK)
	{
		if (is_edit_mode == 0) return; // 수정 모드가 아니면 차단
		switch (edit_field)
		{
			case EDIT_FIELD_YEAR:
			date_time.year++;
			if (date_time.year > 2099) date_time.year = 2000;
			break;
			
			case EDIT_FIELD_MONTH:
			date_time.month++;
			if (date_time.month > 12) date_time.month = 1;
			break;
			
			case EDIT_FIELD_DAY:
			date_time.day++;
			if (date_time.day > 31) date_time.day = 1;
			break;
			
			case EDIT_FIELD_HOUR:
			date_time.hour++;
			if (date_time.hour > 23) date_time.hour = 0;
			break;
			
			case EDIT_FIELD_MINUTE:
			date_time.minute++;
			if (date_time.minute > 59) date_time.minute = 0;
			break;
			
			case EDIT_FIELD_SECOND:
			date_time.second++;
			if (date_time.second > 59) date_time.second = 0;
			break;
			
			default: break;
		}
	}
	else
	{
		cal_backspace(); // 한 바이트 지우기 처리 함수 호출
	}
	
	should_render_display = 1;
}

// [사이드 버튼 2] 시계 모드: 수정 필드 순환 시프트 / 계산기 모드: 소괄호 열기 '(' 주입
static void handle_button_2(void)
{
	if (display_mode == MODE_CLOCK)
	{
		if (is_edit_mode == 0) return;
		
		edit_field = (edit_field + 1) % EDIT_FIELD_MAX; // 연->월->일->시->분->초 순환
	}
	else
	{
		cal_open_paren(); // 수식 문자열 배열에 '(' 문자 적재
	}
	
	should_render_display = 1;
}

// [사이드 버튼 3] 시계 모드: 수정 모드 진입 및 저장 탈출 / 계산기 모드: 소괄호 닫기 ')' 주입
static void handle_button_3(void)
{
	if (display_mode == MODE_CLOCK)
	{
		if (is_edit_mode == 0)
		{
			ds1307_get_time(&date_time); // 수정 진입 직전 RTC 최신 시각 스냅샷 확보
			is_edit_mode = 1;
			edit_field = EDIT_FIELD_YEAR; // 최초 진입 위치는 '연도' 고정
		}
		else
		{
			ds1307_set_time(&date_time); // 완료 후 수정한 누적 본을 하드웨어 RTC에 최종 플래싱 저장
			is_edit_mode = 0;             // 수정 모드 종료 후 탈출
		}
	}
	else
	{
		cal_close_paren(); // 수식 문자열 배열에 ')' 문자 적재
	}
	
	should_render_display = 1;
}

// [사이드 버튼 4] 시스템 메인 디스플레이 화면 모드 토글 스위치 (시계 ↔ 계산기)
static void handle_button_4(void)
{
	if (is_edit_mode) return; // 시계를 수정 중인 불완전 상태(인터로킹 상황)라면 화면 스위칭 차단
	
	if (display_mode == MODE_CLOCK)
	{
		display_mode = MODE_CALCULATOR; // 계산기 모드로 변환
	}
	else
	{
		display_mode = MODE_CLOCK;      // 시계 모드로 복귀
	}
	
	lcd_clear();             // 모드 전환 시 발생할 수 있는 픽셀 잔상 초기화 용도
	
	should_render_display = 1; // 화면 즉시 드로잉 요청 트리거
}