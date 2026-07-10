#ifndef CAL_H_
#define CAL_H_

#include <avr/io.h>

void cal_init(void);
void cal_input_key(uint8_t key);
void cal_reset(void);
void cal_backspace(void);
void cal_open_paren(void);
void cal_close_paren(void);
void cal_get_display_strings(char *line1, char *line2);

#endif /* CAL_H_ */