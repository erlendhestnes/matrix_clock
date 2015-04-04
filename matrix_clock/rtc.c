/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t seconds = 0;
volatile uint8_t minutes = 0;
volatile uint8_t hours = 0;

char str_min[2];
char str_min_prev[2];

char str_hour[2];
char str_hour_prev[2];

void rtc_setup(void) {
	CCP = CCP_IOREG_gc;
	CLK.RTCCTRL = CLK_RTCSRC_TOSC32_gc | CLK_RTCEN_bm;
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	
	//RTC.PER = 32768;
	RTC.PER = 100;
	RTC.INTCTRL = RTC_OVFINTLVL_MED_gc;
	RTC.CNT = 0;
	RTC.COMP = 0;
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;
}

void init_time(void) {
	
	ht1632c_clearScreen();
	
	hours = 18;
	minutes = 4;
	
	str_min[0] = str_min_prev[0] = '0';
	str_min[1] = str_min_prev[1] = '4';
	
	str_hour[0] = str_hour_prev[0] = '1';
	str_hour[1] = str_hour_prev[1] = '8';
	
	ht1632c_drawChar(2,0,'1',1,1);
	ht1632c_drawChar(9,0,'8',1,1);
	ht1632c_drawChar(2,9,'0',1,1);
	ht1632c_drawChar(9,9,'4',1,1);
	ht1632c_writeScreen();
}

void update_time(void) 
{	
	if (seconds == 59) {
		sprintf(str_min, "%d", minutes);
		if (minutes < 10) {
			char temp = str_min[0];
			str_min[0] = '0';
			str_min[1] = temp;
		}
		
		ht1632c_drawChar(2,9,str_min_prev[0],0,1);
		ht1632c_drawChar(9,9,str_min_prev[1],0,1);
		
		ht1632c_drawChar(2,9,str_min[0],1,1);
		ht1632c_drawChar(9,9,str_min[1],1,1);
		
		str_min_prev[0] = str_min[0];
		str_min_prev[1] = str_min[1];
		
		ht1632c_writeScreen();
	}
	
	if (minutes == 59) {
		sprintf(str_hour, "%d", hours);
		if (hours < 10) {
			char temp = str_hour[0];
			str_hour[0] = '0';
			str_hour[1] = temp;
		}
		ht1632c_drawChar(2,0,str_hour_prev[0],0,1);
		ht1632c_drawChar(9,0,str_hour_prev[1],0,1);
		
		ht1632c_drawChar(2,0,str_hour[0],1,1);
		ht1632c_drawChar(9,0,str_hour[1],1,1);
		
		str_hour_prev[0] = str_hour[0];
		str_hour_prev[1] = str_hour[1];
		
		ht1632c_writeScreen();
	}
	
}

ISR(RTC_OVF_vect) {
	seconds++;
	minutes += seconds / 60;
	seconds %= 60;
	hours += minutes / 60;
	minutes %= 60;
}
