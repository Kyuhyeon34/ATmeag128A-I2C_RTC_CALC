#ifndef DS1307_H_
#define DS1307_H_

#include "rtc.h"

#define SLAVE_ADDR			0x68
#define SLAVE_WRITE_ADDR	((SLAVE_ADDR << 1) | 0)
#define SLAVE_READ_ADDR		((SLAVE_ADDR << 1) | 1)

void ds1307_init(date_time_t *initial_time);
void ds1307_get_time(date_time_t *time_data);
void ds1307_set_time(const date_time_t *time_data);

#endif /* DS1307_H_ */