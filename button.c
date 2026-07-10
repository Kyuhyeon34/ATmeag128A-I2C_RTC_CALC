/**
* @file    button.c
* @brief   DDRD/PIND 기반 물리 핀 격리 및 릴리즈 엣지 디텍션 디바운싱 알고리즘 구현
* @date    2026-06-11
* @author  kccistc
*/

#include "button.h"

static const uint8_t button_pins[BUTTON_COUNT] = { BUTTON_0_PIN, BUTTON_1_PIN, BUTTON_2_PIN, BUTTON_3_PIN, BUTTON_4_PIN };

void button_init(void)
{
	BUTTON_DDR &= ~((1 << button_pins[BUTTON_0]) |
	(1 << button_pins[BUTTON_1]) |
	(1 << button_pins[BUTTON_2]) |
	(1 << button_pins[BUTTON_3]) |
	(1 << button_pins[BUTTON_4]));
}

int get_button_state(button_id_t button_id)
{
	static int button_last_status[BUTTON_COUNT] = { BUTTON_RELEASE };
	
	int pin_num = button_pins[button_id];
	int current_state = (BUTTON_PIN & (1 << pin_num)) >> pin_num;

	if (current_state == BUTTON_PRESS && button_last_status[button_id] == BUTTON_RELEASE)
	{
		_delay_ms(15);
		button_last_status[button_id] = BUTTON_PRESS;
		return 0;
	}
	else if (current_state == BUTTON_RELEASE && button_last_status[button_id] == BUTTON_PRESS)
	{
		_delay_ms(15);
		button_last_status[button_id] = BUTTON_RELEASE;
		return 1;
	}
	
	return 0;
}
