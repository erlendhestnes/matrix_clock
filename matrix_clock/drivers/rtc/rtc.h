/*
 * rtc.h
 *
 * Created: 10/26/2014 10:12:14 AM
 *  Author: Administrator
 */ 

#ifndef _RTC_H_
#define _RTC_H_

#include "../../global.h"

#define TOP_HALF		9
#define BOTTOM_HALF		0

void rtc_setup(void);
void rtc_show_hours(void);
void rtc_show_minutes(void);
void rtc_enable_time_render(void);
void rtc_disable_time_render(void);
void rtc_update_display(uint8_t pos, uint8_t time);
void rtc_force_update(void);

#endif