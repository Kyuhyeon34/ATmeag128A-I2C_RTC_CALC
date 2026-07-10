/*
* I2C_RTC.h
*
* Created: 2026-07-01 오후 3:06:33
*  Author: kccistc
*/


#ifndef I2C_RTC_H_
#define I2C_RTC_H_

#include <avr/io.h>

#define I2C_SCL		PORTD0
#define I2C_SDA		PORTD1

void I2C_init(void);				// I2C 초기화
void I2C_start(void);				// I2C 시작
void I2C_transmit(uint8_t data);	// 1바이트 전송
uint8_t I2C_receive_ACK(void);		// 1바이트 수신 & ACK
uint8_t I2C_receive_NACK(void);		// 1바이트 수신 & NACK
void I2C_stop(void);				// I2C 정지

#endif /* I2C_RTC_H_ */