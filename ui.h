// ==========================================================================
// 디스플레이 렌더링 및 모달 버튼 이벤트 통합 관리 헤더 (ui.h)
// ==========================================================================
#ifndef UI_H_
#define UI_H_

#include "rtc.h"
#include <stdint.h>

// 시스템 동작 화면 모드
typedef enum {
	MODE_CLOCK,
	MODE_CALCULATOR
} display_mode_t;

extern volatile uint8_t			is_edit_mode;
extern volatile edit_field_t	edit_field;
extern volatile uint8_t			should_render_display;
extern date_time_t				date_time;
extern volatile display_mode_t	display_mode;

void ui_init(void);
void ui_update(void);
void ui_handle_button(uint8_t button_id);

#endif // UI_H_