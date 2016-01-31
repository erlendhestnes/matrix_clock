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

#define PROXIMITY_THRESHOLD 1600
#define MENU_TIMEOUT 15000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef enum {
	Monday = 1,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
	Sunday,
	NO_DAY
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
	December,
	NO_MONTH
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
	int8_t timezone;
	uint8_t DST;
} time_env_t;

typedef struct {
	uint8_t minutes;
	uint8_t hours;
	uint8_t sound;
} alarm_t;

typedef struct {
	uint16_t id;
	uint32_t runtime;
	char name[25];
	time_env_t time;
	char temperature[3];
	char weather_info[40];
	char location[25];
	int8_t brightness;
	uint16_t ps1;
	char wifi_pswd[25];
	char wifi_ssid[25];
	uint8_t menu_id;
	alarm_t alarm;
	uint16_t baseline[3];
} env_variables_t;

env_variables_t env_var;

#define SHOW_MANUAL
#define DEBUG_ON

#define CLOCK_NAME "MARK"
#define CLOCK_ID   1

//Delay implementation that accepts dynamic parameters
static inline void delay_ms( int ms )
{
	for (int i = 0; i < ms; i++)
	{
		_delay_ms(1);
	}
}

static inline char reverse_byte(char b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

static inline void reverse_string( char *s, int l )
{
	int i, j;
	char c;

	for (i = 0, j = l-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

static inline char* itoa_simple( char *s, long num ) {
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

	reverse_string( rev, s - rev);

	return (char*)(s - begin);
}

#endif