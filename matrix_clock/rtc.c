/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

time_t time;

#define TOP		0
#define BOTTOM  9

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
	ht1632c_clear_screen();
	
	time.hours = 0;
	time.minutes = 0;
	time.seconds = 0;
	time.days = 0;
	time.years = 0;
	
	int_to_string(BOTTOM,time.minutes);
	int_to_string(TOP,time.hours);
}

void set_time(time_t *t) {
	
	cli();
	
	time.hours = t->hours;
	time.minutes = t->minutes;
	time.seconds = t->seconds;
	time.days = t->days;
	time.years = t->years;
	
	int_to_string(BOTTOM,time.minutes);
	int_to_string(TOP,time.hours);
	
	sei();
}

void update_time(void) {
	if (time.seconds == 59) {
		int_to_string(BOTTOM,time.minutes);
	}
	if (time.minutes == 59) {
		int_to_string(TOP,time.hours);
	}
}

void int_to_string(uint8_t pos, uint8_t t) {
	
	char buffer[2];
	char temp;
	
	sprintf(buffer, "%d", t);
	
	/*
	if (t < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	*/

	ht1632c_fill_rect(pos,0,8,16,0);
	
	//Write new numbers
	ht1632c_draw_char(2,pos,buffer[0],1,1);
	ht1632c_draw_char(9,pos,buffer[1],1,1);
	
	ht1632c_refresh_screen();
}

ISR(RTC_OVF_vect) {
	time.seconds++;
	time.minutes += time.seconds / 60;
	time.seconds %= 60;
	time.hours += time.minutes / 60;
	time.minutes %= 60;
}
