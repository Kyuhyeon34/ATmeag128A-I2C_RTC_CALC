/**
* @file    button.h
* @brief   ATmega128A 다중 스위치 입력 제어 및 상태 스캔 인터페이스
* @date    2026-06-11
* @author  kccistc
*/

#ifndef BUTTON_H_
#define BUTTON_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define BUTTON_DDR      DDRD
#define BUTTON_PIN      PIND

#define BUTTON_PRESS    1
#define BUTTON_RELEASE  0

#define BUTTON_0_PIN    3
#define BUTTON_1_PIN    4
#define BUTTON_2_PIN    5
#define BUTTON_3_PIN    6
#define BUTTON_4_PIN    7

typedef enum {
	BUTTON_0 = 0,
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_COUNT
} button_id_t;

void button_init(void);
int get_button_state(button_id_t button_id);

#endif /* BUTTON_H_ */