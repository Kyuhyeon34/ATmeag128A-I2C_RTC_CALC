// ==========================================================================
// DS1307 RTC 시계 칩 제어 드라이버 구현부 (ds1307.c)
// ==========================================================================

#include "ds1307.h"
#include "I2C_RTC.h"

// BCD 형식을 10진수로 변환 (RTC 데이터 읽기용)
static uint8_t bcd_to_decimal(uint8_t bcd)
{
	return (bcd >> 4) * 10 + (bcd & 0x0F);
}

// 10진수를 BCD 형식으로 변환 (RTC 데이터 쓰기용)
static uint8_t decimal_to_bcd(uint8_t decimal)
{
	return ((decimal / 10) << 4) | (decimal % 10);
}

// RTC 칩 초기화 및 배터리 백업 판단 함수
void ds1307_init(date_time_t *initial_time)
{
	I2C_init(); // I2C 하드웨어 모듈 가동
	
	// 1. 현재 내부 포인터를 00h(초) 레지스터 위치로 이동
	I2C_start();
	I2C_transmit(SLAVE_WRITE_ADDR);		// 쓰기 주소 전송
	I2C_transmit(0);					// 0번 주소방 지정
	I2C_stop();
	
	// 2. 00h번지(초) 레지스터 값 1바이트 읽기
	I2C_start();
	I2C_transmit(SLAVE_READ_ADDR); // 읽기 주소 전송
	uint8_t second = I2C_receive_NACK();
	I2C_stop();
	
	// 3. 7번 비트(CH) 검사를 통한 시계 상태 판별
	if (second & 0x80)
	{
		// CH 비트가 1이면 시계가 멈춘 상태 -> 초기 시간 세팅 및 오실레이터 가동
		ds1307_set_time(initial_time);
	}
	else
	{
		// CH 비트가 0이면 정상 구동 중 상태 -> 배터리로 가고 있던 진짜 시간을 변수에 역복사
		ds1307_get_time(initial_time);
	}
}

// 하드웨어 RTC에서 실시간으로 시각 데이터를 읽어오는 함수
void ds1307_get_time(date_time_t *time_data)
{
	// 1. 읽어올 시작 메모리 주소를 0번지(초)로 강제 지정
	I2C_start();
	I2C_transmit(SLAVE_WRITE_ADDR);
	I2C_transmit(0);
	I2C_stop();
	
	// 2. 읽기 모드로 다시 접근하여 데이터 순차 수신
	I2C_start();
	I2C_transmit(SLAVE_READ_ADDR);
	
	// 칩 내부 주소가 자동 증가하므로 연속 수신 가능 (ACK로 응답)
	uint8_t second  = I2C_receive_ACK();		// 00h: 초
	uint8_t minute  = I2C_receive_ACK();		// 01h: 분
	uint8_t hour = I2C_receive_ACK();			// 02h: 시
	uint8_t day_of_week = I2C_receive_ACK();	// 03h: 요일 (스킵)
	uint8_t day  = I2C_receive_ACK();			// 04h: 일
	uint8_t month  = I2C_receive_ACK();			// 05h: 월
	uint8_t year = I2C_receive_NACK();			// 06h: 년 (마지막이므로 NACK 전송)
	I2C_stop();
	
	// 3. 수신된 BCD 데이터를 10진수로 변환하여 메인 구조체에 탑재
	time_data->second		= bcd_to_decimal(second & 0x7F);	// CH 비트 제외 필터링
	time_data->minute		= bcd_to_decimal(minute);
	time_data->hour			= bcd_to_decimal(hour & 0x3F);		// 24시간 모드 비트 필터링
	time_data->day_of_week	= bcd_to_decimal(day_of_week);
	time_data->day			= bcd_to_decimal(day);
	time_data->month		= bcd_to_decimal(month);
	time_data->year			= 2000 + bcd_to_decimal(year);		// 2자리 연도를 4자리 연도로 가공
}

// 사용자가 설정한 변경 시간을 하드웨어 RTC에 덮어쓰는 함수
void ds1307_set_time(const date_time_t *time_data)
{
	// 1. 기록할 시작 메모리 주소를 0번지(초)로 지정
	I2C_start();
	I2C_transmit(SLAVE_WRITE_ADDR);
	I2C_transmit(0);
	
	// 2. 10진수 시간 데이터를 BCD로 포맷팅하여 순서대로 전송
	I2C_transmit(decimal_to_bcd(time_data->second) & 0x7F); // 7번 비트를 0으로 밀어 시계 가동(CH=0)
	I2C_transmit(decimal_to_bcd(time_data->minute));
	I2C_transmit(decimal_to_bcd(time_data->hour) & 0x3F);   // 24시간 모드로 강제 지정
	I2C_transmit(decimal_to_bcd(time_data->day_of_week));
	I2C_transmit(decimal_to_bcd(time_data->day));
	I2C_transmit(decimal_to_bcd(time_data->month));
	I2C_transmit(decimal_to_bcd(time_data->year % 100));    // 4자리 연도를 하위 2자리 숫자로 압축 변환
	
	I2C_stop(); // 정지 신호로 쓰기 공정 마감
}