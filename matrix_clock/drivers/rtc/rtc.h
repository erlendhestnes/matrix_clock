/*
 * rtc.h
 *
 * Created: 10/26/2014 10:12:14 AM
 *  Author: Administrator
 */ 

#ifndef _RTC_H_
#define _RTC_H_

#include "../../global.h"

void rtc_setup(void);
void rtc_init_time(void);

void rtc_set_time(uint8_t s,
uint8_t m,
uint8_t h,
uint16_t d,
uint16_t y);

void rtc_update_display(uint8_t pos, uint8_t t);
void rtc_update_display_alt(void);

void rtc_refresh_display(void);

void rtc_increment_hour(void);
void rtc_decrement_hour(void);
void rtc_increment_minute(void);
void rtc_decrement_minute(void);
void rtc_increment_second(void);
void rtc_decrement_second(void);
void rtc_increment_day(void);
void rtc_decrement_day(void);
void rtc_increment_month(void);
void rtc_decrement_month(void);
void rtc_increment_year(void);
void rtc_decrement_year(void);

void rtc_set_time_mode(void);
void rtc_time_mode(void);

void rtc_show_hours(void);
void rtc_show_minutes(void);

void rtc_disable_time_render(void);
void rtc_enable_time_render(void);

const char* get_day_name(weekdays_t day);
const char* get_month_name(months_t month);
uint8_t get_month_number(char *month);
uint8_t get_days_in_month(months_t month, uint16_t year);

#endif