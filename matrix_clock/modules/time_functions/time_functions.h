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
uint8_t time_get_weekday(uint8_t d, uint8_t m, uint16_t y);
uint16_t time_get_days_in_year(uint8_t d, uint8_t m, uint16_t y);
int time_get_weeknumber(uint8_t day, uint8_t month, uint16_t year);
bool time_is_leap_year(uint16_t y);
#endif /* TIME_FUNCTIONS_H_ */