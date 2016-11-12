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

#define PROXIMITY_THRESHOLD2 1000
#define PROXIMITY_THRESHOLD 360
#define IR_BRIGHTNESS_THRESHOLD 300
#define MENU_TIMEOUT 12000

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
#include <avr/eeprom.h>

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
} time_env_t;

typedef struct {
	uint8_t minutes;
	uint8_t hours;
	uint8_t sound;
} alarm_t;

typedef struct {
	uint8_t id;
	uint8_t menu_id;
	uint8_t battery;
	int8_t brightness;
	uint16_t ps1;
	uint16_t baseline[3];
	uint32_t runtime;
	char name[25];
	char temperature[3];
	char weather_info[40];
	char city[25];
	char wifi_pswd[25];
	char wifi_ssid[25];
	alarm_t alarm;
	time_env_t time;
} env_variables_t;

env_variables_t env;

#define SHOW_MANUAL
#define DEBUG_ON
#define WIFI_ON
//#define IR_SLIDER_ALGORITHM

#define CLOCK_NAME "MARK"
#define CLOCK_ID   1

//Delay implementation that accepts dynamic parameters
static inline void delay_ms(int ms)
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
		n = -num;
		*s++ = '-';
		rev++;
	}

	do {       
		*s++ = (n % 10) + '0';   
	} while ((n /= 10) > 0); 

	reverse_string( rev, s - rev);

	return (char*)(s - begin);
}

static uint8_t read_signature_byte(uint16_t Address) {
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	__asm__ ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}

#endif