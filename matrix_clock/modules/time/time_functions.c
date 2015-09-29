/*
 * time_functions.c
 *
 * Created: 9/29/2015 5:45:42 PM
 *  Author: Administrator
 */ 

#include "time_functions.h"

const char* time_get_day_name(weekdays_t day)
{
	switch (day)
	{
		case Monday: return "MON";
		case Tuesday: return "TUE";
		case Wednesday: return "WED";
		case Thursday: return "THU";
		case Friday: return "FRI";
		case Saturday: return "SAT";
		case Sunday: return "SUN";
	}
}

const char* time_get_month_name(months_t month)
{
	switch (month)
	{
		case January: return "JAN";
		case Februrary: return "FEB";
		case March: return "MAR";
		case April: return "APR";
		case May: return "MAY";
		case Juni: return "JUN";
		case Juli: return "JUL";
		case August: return "AUG";
		case September: return "SEP";
		case October: return "OCT";
		case November: return "NOV";
		case December: return "DEC";
	}
}

uint8_t time_get_month_number(char *month)
{
	if (strstr(month,"Jan")) {
		return 1;
		} else if (strstr(month,"Feb")) {
		return 2;
		} else if (strstr(month,"Mar")) {
		return 3;
		} else if (strstr(month,"Apr")) {
		return 4;
		} else if (strstr(month,"May")) {
		return 5;
		} else if (strstr(month,"Jun")) {
		return 6;
		} else if (strstr(month,"Jul")) {
		return 7;
		} else if (strstr(month,"Aug")) {
		return 8;
		} else if (strstr(month,"Sep")) {
		return 9;
		} else if (strstr(month,"Oct")) {
		return 10;
		} else if (strstr(month,"Nov")) {
		return 11;
		} else if (strstr(month,"Dec")) {
		return 12;
		} else {
		return 0;
	}
}

uint8_t time_get_days_in_month(months_t month, uint16_t year)
{
	switch (month)
	{
		case January: return 31;
		case Februrary:
		if (is_leap_year(year))
		return 29;
		else
		return 28;
		case March: return 31;
		case April: return 30;
		case May: return 31;
		case Juni: return 30;
		case Juli: return 31;
		case August: return 31;
		case September: return 30;
		case October: return 31;
		case November: return 30;
		case December: return 31;
	}
}

int time_get_day_of_week(int d, int m, int y)
{
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	y -= m < 3;
	return ( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}