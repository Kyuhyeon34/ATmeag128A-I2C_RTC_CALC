/*
* keypad.c
*
* Created: 2026-06-29 오후 2:11:23
*  Author: kccistc
*/

#include "keypad.h"
#include <stdio.h>

static uint8_t get_button(uint8_t row, uint8_t col)
{
	static const uint8_t keypad_char[4][4] = {
		{' ', '0', '=', '+'},
		{'4', '5', '6', '*'},
		{'7', '8', '9', '-'},
		{'1', '2', '3', '/'},
	};
	
	// 각 스위치의 이전 상태 기록 배열 (1: 뗀 상태, 0: 누른 상태)
	static uint8_t prev_state[4][4] = {
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1}
	};
	
	uint8_t current_state = 1;
	
	// 타겟 컬럼만 LOW 출력 설정 (C4 가 0번에 연결되어 있으므로 3 - col)
	KEYPAD_PORT = 0xFF;
	KEYPAD_PORT &= ~(1 << (3 - col));
	
	// 핀 전압 안정화 및 레지스터 동기화를 위한 정밀 딜레이
	for (int delay = 0; delay < 20 ; delay++);

	// 타겟 행의 핀 상태 판독 (상위 4비트)
	current_state = (KEYPAD_PIN & (1 << (row + 4))) >> (row + 4);
	
	// 버튼을 처음 누른 시점 감지
	if (current_state == 0 && prev_state[row][col] == 1)
	{
		prev_state[row][col] = 0;
		return 0;
	}
	// 버튼을 누르고 있다가 완전히 뗀 정격 시점 감지
	else if (current_state == 1 && prev_state[row][col] == 0)
	{
		prev_state[row][col] = 1; // 초기화
		//printf("key: %c\n", keypad_char[row][col]);
		return keypad_char[row][col];
	}
	
	return 0;
}

void keypad_init(void)
{
	KEYPAD_DDR = 0x0F; // row: 입력, col: 출력
	KEYPAD_PORT = 0xFF;
}

uint8_t keypad_scan(void)
{
	uint8_t data = 0; // key 값 저장 변수
	
	for (uint8_t row = 0; row < 4; row++)
	{
		for (uint8_t col = 0; col < 4 ; col++)
		{
			data = get_button(row, col);
			if (data)
			{
				return data;
			}
		}
	}
	
	return 0;
}
