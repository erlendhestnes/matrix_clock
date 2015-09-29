/*
 * time_functions.h
 *
 * Created: 9/29/2015 5:45:26 PM
 *  Author: Administrator
 */ 


#ifndef TIME_FUNCTIONS_H_
#define TIME_FUNCTIONS_H_

#include "../../global.h"

const char* time_get_day_name(weekdays_t day);
const char* time_get_month_name(months_t month);
uint8_t time_get_month_number(char *month);
uint8_t time_get_days_in_month(months_t month, uint16_t year);
int time_get_day_of_week(int d, int m, int y);

void display_alarm_increment_minute(uint8_t min);
void display_alarm_decrement_minute(uint8_t min);
void display_alarm_increment_hour(uint8_t hour);
void display_alarm_decrement_hour(uint8_t hour);

#endif /* TIME_FUNCTIONS_H_ */