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
		case NO_DAY: return "-";
		default:
			return "-";
	}
	return "-";
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
		default:
			return "-";
	}
	return "-";
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
	return 0;
}

uint8_t time_get_days_in_month(months_t month, uint16_t year)
{
	switch (month)
	{
		case January: return 31;
		case Februrary:
		if (time_is_leap_year(year))
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
		default:
			return 0;
	}
	return 0;
}

uint8_t time_get_weekday(uint8_t day, uint8_t month, uint16_t year)
{
	int16_t yyyy = year;
	int16_t mm = month;
	int16_t dd = day;

	// Declare other required variables
	int16_t day_of_year_number;
	int16_t jan_1_weekday;
	int16_t weekday;

	int16_t i,j,k,l;
	int16_t mnth[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

	// Set DayofYear Number for yyyy mm dd
	day_of_year_number = dd + mnth[mm-1];

	// Increase of Dayof Year Number by 1, if year is leapyear and month is february
	if ((time_is_leap_year(yyyy) == true) && (mm == 2))
	day_of_year_number += 1;

	// Find the Jan1WeekDay for year
	i = (yyyy - 1) % 100;
	j = (yyyy - 1) - i;
	k = i + i/4;
	jan_1_weekday = 1 + (((((j / 100) % 4) * 5) + k) % 7);

	// Calcuate the WeekDay for the given date
	l= day_of_year_number + (jan_1_weekday - 1);
	weekday = 1 + ((l - 1) % 7);

	return weekday;
}

uint16_t time_get_days_in_year(uint8_t day, uint8_t month, uint16_t year)
{
	uint16_t days_count = 0;
	
	for (uint8_t i = 1; i < month; i++) {
		days_count += time_get_days_in_month(i,year);
	}
	days_count += day;
	
	return days_count;
}

// Method to check Leap Year
bool time_is_leap_year(uint16_t year) 
{	    
	if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
		return true;
	else
		return false;
}

// Static Method to return ISO WeekNumber (1-53) for a given year
int16_t time_get_weeknumber(uint8_t day, uint8_t month, uint16_t year) 
{	    
	int16_t yyyy = year;
	int16_t mm= month;
	int16_t dd = day;

	// Declare other required variables
	int16_t day_of_year_number;
	int16_t jan_1_weekday;
	int16_t week_number = 0, weekday;
	    
	int16_t i,j,k,l,m,n;
	int16_t mnth[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

	int16_t year_number;
	    
	// Set DayofYear Number for yyyy mm dd
	day_of_year_number = dd + mnth[mm-1];

	// Increase of Dayof Year Number by 1, if year is leapyear and month is february
	if ((time_is_leap_year(yyyy) == true) && (mm == 2))
	day_of_year_number += 1;

	// Find the Jan1WeekDay for year
	i = (yyyy - 1) % 100;
	j = (yyyy - 1) - i;
	k = i + i/4;
	jan_1_weekday = 1 + (((((j / 100) % 4) * 5) + k) % 7);

	// Calcuate the WeekDay for the given date
	l = day_of_year_number + (jan_1_weekday - 1);
	weekday = 1 + ((l - 1) % 7);

	// Find if the date falls in YearNumber set WeekNumber to 52 or 53
	if ((day_of_year_number <= (8 - jan_1_weekday)) && (jan_1_weekday > 4))
	{
		year_number = yyyy - 1;
		if ((jan_1_weekday == 5) || ((jan_1_weekday == 6) && (jan_1_weekday > 4)))
		week_number = 53;
		else
		week_number = 52;
	}
	else
	
	year_number = yyyy;
	    
	// Set WeekNumber to 1 to 53 if date falls in YearNumber
	if (year_number == yyyy)
	{
		if (time_is_leap_year(yyyy)==true)
		m = 366;
		else
		m = 365;
		if ((m - day_of_year_number) < (4-weekday))
		{
			year_number = yyyy + 1;
			week_number = 1;
		}
	}
	    
	if (year_number==yyyy) {
		n=day_of_year_number + (7 - weekday) + (jan_1_weekday -1);
		week_number = n / 7;
		if (jan_1_weekday > 4)
		week_number -= 1;
	}

	return (week_number);
}
