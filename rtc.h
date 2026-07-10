/**
* @file    rtc.h
* @brief   날짜 및 시간 데이터 구조체와 시스템 상태 제어 변수 선언
*/

#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

typedef struct {
	uint8_t second;			// 초 (0~59)
	uint8_t minute;			// 분 (0~59)
	uint8_t hour;			// 시 (0~23)
	uint8_t day_of_week;	// 요일 (1:일, 2:월, 3:화, 4:수, 5:목, 6:금, 7:토)
	uint8_t day;			// 일 (1~31)
	uint8_t month;			// 월 (1~12)
	uint16_t year;			// 연도 (2000~2099)
} date_time_t;

typedef enum {
	EDIT_FIELD_YEAR = 0,
	EDIT_FIELD_MONTH,
	EDIT_FIELD_DAY,
	EDIT_FIELD_HOUR,
	EDIT_FIELD_MINUTE,
	EDIT_FIELD_SECOND,
	EDIT_FIELD_MAX
} edit_field_t;

extern volatile uint8_t      is_edit_mode;
extern volatile edit_field_t edit_field;
extern volatile uint8_t      should_render_display;
extern date_time_t           date_time;

#endif /* RTC_H_ */