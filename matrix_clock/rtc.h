/*
 * rtc.h
 *
 * Created: 10/26/2014 10:12:14 AM
 *  Author: Administrator
 */ 

#ifndef _RTC_H_
#define _RTC_H_

#include <stdint.h>

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint16_t days;
	uint16_t years;
}time_t;

void rtc_setup(void);
void init_time(void);
void set_time(time_t *t);
void update_time(void);
void int_to_string(uint8_t pos, uint8_t t);

#endif