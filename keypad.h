/*
* keypad.h
*
* Created: 2026-06-29 오후 2:11:15
*  Author: kccistc
*/


#ifndef KEYPAD_H_
#define KEYPAD_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define KEYPAD_DDR	DDRA
#define KEYPAD_PIN	PINA
#define KEYPAD_PORT	PORTA

void keypad_init(void);
uint8_t keypad_scan(void);

#endif /* KEYPAD_H_ */