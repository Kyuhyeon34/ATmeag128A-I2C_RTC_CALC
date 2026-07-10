/*
* I2C_RTC.c
*
* Created: 2026-07-01 오후 3:06:17
*  Author: kccistc
*/

#include "I2C_RTC.h"

void I2C_init(void)
{
	DDRD |= (1 << I2C_SCL);
	DDRD |= (1 << I2C_SDA);
	
	TWBR = 32; // I2C 클록 주파수 설정 200KHz
}

void I2C_start(void)
{
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;
	
	while ( !(TWCR & (1 << TWINT)) ); // 시작 완료 대기
}

void I2C_transmit(uint8_t data)
{
	TWDR = data;
	TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA;
	
	while ( !(TWCR & (1 << TWINT)) ); // 전송 완료 대기
}

uint8_t I2C_receive_ACK(void)
{
	TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA;
	
	while ( !(TWCR & (1 << TWINT)) ); // 수신 완료 대기
	
	return TWDR;
}

uint8_t I2C_receive_NACK(void)
{
	TWCR = 1 << TWINT | 1 << TWEN;
	
	while ( !(TWCR & (1 << TWINT)) ); // 수신 완료 대기
	
	return TWDR;
}

void I2C_stop(void)
{
	TWCR = 1 << TWINT | 1 << TWSTO | 1 << TWEN;
}