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
	uint8_t s;
	uint8_t m;
	uint8_t h;
	uint8_t d;
	uint8_t y;
}time_t;

void rtc_setup(void);
void init_time(void);
void update_time(void);

#endif