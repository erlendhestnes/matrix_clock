/*
 * rtc.h
 *
 * Created: 10/26/2014 10:12:14 AM
 *  Author: Administrator
 */ 

#ifndef _RTC_H_
#define _RTC_H_

#include "global.h"

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint16_t days;
	uint8_t weeks;
	uint16_t years;
}time_t;

void rtc_setup(void);
void rtc_init_time(void);
void rtc_set_time(uint8_t h, uint8_t m, uint8_t s);
void rtc_update_display(uint8_t pos, uint8_t t);
void rtc_update_display_alt(void);

void rtc_increment_hour(void);
void rtc_decrement_hour(void);
void rtc_increment_minute(void);
void rtc_decrement_minute(void);

void rtc_set_time_mode(void);
void rtc_time_mode(void);

#endif