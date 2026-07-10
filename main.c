// ==========================================================================
// 시스템 명칭: I2C 기반 RTC 시계 및 소괄호 사칙연산 계산기 통합 시스템
// 메인 칩셋  : ATmega128A (F_CPU = 16MHz External Crystal)
// 구동 사상  : 1ms 마스터 타이머 기반 비차단 스케줄링 (Multi-Tasking)
// ==========================================================================

#define F_CPU 16000000UL

#include "button.h"
#include "keypad.h"
#include "cal.h"
#include "ds1307.h"
#include "timer0.h"
#include "ui.h"

#include <avr/interrupt.h>

void task_process_buttons(void);
void task_process_keypad(void);
void task_sync_rtc(void);
void task_render_display(void);

int main(void)
{
	timer0_init();
	button_init();
	keypad_init();
	cal_init();
	ds1307_init(&date_time);
	ui_init();
	
	sei();
	
	while (1)
	{
		// 버튼 처리
		task_process_buttons();
		
		// 시간 동기화
		task_sync_rtc();
		
		// 키패드 입력 처리
		task_process_keypad();
		
		// 디스플레이 표출
		task_render_display();
	}
}

void task_process_buttons(void)
{
	static button_id_t button_id;
	static uint8_t i;
	
	// 사이드 푸시 버튼 실시간 감시 및 매핑 함수 호출
	for(i = 0; i < BUTTON_COUNT; i++)
	{
		button_id = (button_id_t)i;
		if (get_button_state(button_id))
		{
			ui_handle_button((uint8_t)button_id);
			break;
		}
	}
}

void task_process_keypad(void)
{
	static uint32_t last_keypad_sync_ms = 0;
	
	// 60ms 주기 키패드 스캔
	if (ms_count - last_keypad_sync_ms >= 60)
	{
		last_keypad_sync_ms = ms_count;
		
		if (display_mode == MODE_CALCULATOR)
		{
			uint8_t keypad_data = keypad_scan();
			if (keypad_data != 0)
			{
				cal_input_key(keypad_data);
				should_render_display = 1; // 계산기 모드일 때만 키 입력 시 화면 리프레시
			}
		}
	}
}

void task_sync_rtc(void)
{
	static date_time_t new_date_time;
	static uint32_t last_rtc_sync_ms = 0;
	
	if (is_edit_mode) return;
	
	if (ms_count - last_rtc_sync_ms >= 200)
	{
		last_rtc_sync_ms = ms_count;
		
		ds1307_get_time(&new_date_time); // 하드웨어 RTC 칩에서 실시간 시간 획득
		
		// 새로 읽은 초(second)가 기존 메모리 값과 다를 때만 갱신
		if (new_date_time.second != date_time.second)
		{
			date_time = new_date_time;
			
			if (display_mode == MODE_CLOCK)
			{
				should_render_display = 1;
			}
		}
	}
}

void task_render_display(void)
{
	// 최종 결과 LCD 화면 드로잉 실행 유닛
	if (should_render_display)
	{
		should_render_display = 0;	// 플래그 청소
		ui_update();				// 화면 갱신
	}
}