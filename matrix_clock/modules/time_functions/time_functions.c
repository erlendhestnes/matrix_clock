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
	}
}

uint8_t time_get_weekday(uint8_t d, uint8_t m, uint16_t y)
{
	// Set Year
	int yyyy = y;

	// Set Month
	int mm = m;
	
	// Set Day
	int dd = d;

	// Declare other required variables
	int DayOfYearNumber;
	int Jan1WeekDay;
	int WeekDay;

	int i,j,k,l;
	int Mnth[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

	// Set DayofYear Number for yyyy mm dd
	DayOfYearNumber = dd + Mnth[mm-1];

	// Increase of Dayof Year Number by 1, if year is leapyear and month is february
	if ((time_is_leap_year(yyyy) == true) && (mm == 2))
	DayOfYearNumber += 1;

	// Find the Jan1WeekDay for year
	i = (yyyy - 1) % 100;
	j = (yyyy - 1) - i;
	k = i + i/4;
	Jan1WeekDay = 1 + (((((j / 100) % 4) * 5) + k) % 7);

	// Calcuate the WeekDay for the given date
	l= DayOfYearNumber + (Jan1WeekDay - 1);
	WeekDay = 1 + ((l - 1) % 7);

	return WeekDay;
}

uint16_t time_get_days_in_year(uint8_t d, uint8_t m, uint16_t y)
{
	uint16_t days_count = 0;
	
	for (uint8_t i = 1; i < m; i++) {
		days_count += time_get_days_in_month(i,y);
	}
	days_count += d;
	return days_count;
}

// Method to check Leap Year
bool time_is_leap_year(uint16_t y) {
	    
	if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
		return true;
	else
		return false;
}

// Static Method to return ISO WeekNumber (1-53) for a given year
int time_get_weeknumber(uint8_t day, uint8_t month, uint16_t year) {
	    
	// Set Year
	int yyyy = year;

	// Set Month
	int mm= month;
	    
	// Set Day
	int dd = day;

	// Declare other required variables
	int DayOfYearNumber;
	int Jan1WeekDay;
	int WeekNumber=0, WeekDay;
	    
	int i,j,k,l,m,n;
	int Mnth[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

	int YearNumber;
	    
	// Set DayofYear Number for yyyy mm dd
	DayOfYearNumber = dd + Mnth[mm-1];

	// Increase of Dayof Year Number by 1, if year is leapyear and month is february
	if ((time_is_leap_year(yyyy) == true) && (mm == 2))
	DayOfYearNumber += 1;

	// Find the Jan1WeekDay for year
	i = (yyyy - 1) % 100;
	j = (yyyy - 1) - i;
	k = i + i/4;
	Jan1WeekDay = 1 + (((((j / 100) % 4) * 5) + k) % 7);

	// Calcuate the WeekDay for the given date
	l= DayOfYearNumber + (Jan1WeekDay - 1);
	WeekDay = 1 + ((l - 1) % 7);

	// Find if the date falls in YearNumber set WeekNumber to 52 or 53
	if ((DayOfYearNumber <= (8 - Jan1WeekDay)) && (Jan1WeekDay > 4))
	{
		YearNumber = yyyy - 1;
		if ((Jan1WeekDay == 5) || ((Jan1WeekDay == 6) && (Jan1WeekDay > 4)))
		WeekNumber = 53;
		else
		WeekNumber = 52;
	}
	else
	YearNumber = yyyy;
	    

	// Set WeekNumber to 1 to 53 if date falls in YearNumber
	if (YearNumber == yyyy)
	{
		if (time_is_leap_year(yyyy)==true)
		m = 366;
		else
		m = 365;
		if ((m - DayOfYearNumber) < (4-WeekDay))
		{
			YearNumber = yyyy + 1;
			WeekNumber = 1;
		}
	}
	    
	if (YearNumber==yyyy) {
		n=DayOfYearNumber + (7 - WeekDay) + (Jan1WeekDay -1);
		WeekNumber = n / 7;
		if (Jan1WeekDay > 4)
		WeekNumber -= 1;
	}

	return (WeekNumber);
}
