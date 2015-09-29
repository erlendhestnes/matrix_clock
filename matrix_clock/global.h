/*
 * global.h
 *
 * Created: 4/12/2015 4:01:21 PM
 *  Author: Administrator
 */ 

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define F_CPU 32000000UL

#define RAND_MAX 255

#define PROXIMITY_THRESHOLD 3200

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
	Sunday,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
} weekdays_t;

typedef enum {
	January = 1,
	Februrary,
	March,
	April,
	May,
	Juni,
	Juli,
	August,
	September,
	October,
	November,
	December
} months_t;

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	weekdays_t weekday;
	uint8_t week;
	months_t month;
	uint16_t year;
} time_env_t;

typedef struct {
	uint8_t minutes;
	uint8_t hours;
} alarm_t;

typedef struct {
	uint16_t id;
	char name[10];
	time_env_t time;
	char temperature[3];
	char weather_info[25];
	int8_t brightness;
	uint16_t ps1;
	char wifi_pswd[20];
	char wifi_ssid[20];
	uint8_t menu_id;
	alarm_t alarm;
} env_variables_t;

env_variables_t env_var;

//#define SHOW_MANUAL
#define DEBUG_ON

#define CLOCK_NAME "MARK"
#define CLOCK_ID   1

static inline bool is_leap_year(int year) {
	if ((year & 3) == 0 && ((year % 25) != 0 || (year & 15) == 0)) {
		return true;
	} else {
		return false;
	}
}

//Delay implementation that accepts dynamic parameters
static inline void delay_ms( int ms )
{
	for (int i = 0; i < ms; i++)
	{
		_delay_ms(1);
	}
}

/**
 * reverse:  reverse first l chars of string s in place
 */
static inline void reverse( char *s, int l )
{
	int i, j;
	char c;

	for (i = 0, j = l-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/**
 * convert n to characters in s
 * s will NOT be zero terminated, return strlen of s
 * this is a simple implementation that works for complete long int range
 * architecture independent
 * about 2 times faster than sprintf (in range [INT_MIN/10 - INT_MAX/10])
 */
static inline void itoa_simple( char *s, long num ) {
	char *begin = s;
	char *rev = s;
	unsigned long n = num;

	if( num < 0 ) {
		 // forget about the sign, don't reverse it later
		n = -num;
		*s++ = '-';
		rev++;
	}

	do {       /* generate digits in reverse order */
		*s++ = (n % 10) + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */

	reverse( rev, s - rev);

	return s - begin;
}

#endif