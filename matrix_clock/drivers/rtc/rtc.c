/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"

time_t time;

#define TOP		9
#define BOTTOM  0

void rtc_setup(void) {
	CCP = CCP_IOREG_gc;
	CLK.RTCCTRL = CLK_RTCSRC_TOSC32_gc | CLK_RTCEN_bm;
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	
	//RTC.PER = 32768;
	RTC.PER = 1000;
	RTC.INTCTRL = RTC_OVFINTLVL_MED_gc;
	RTC.CNT = 0;
	RTC.COMP = 0;
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;
}

void rtc_init_time(void) {
	ht1632c_clear_screen();
	
	time.hours = 0;
	time.minutes = 0;
	time.seconds = 0;
	time.days = 0;
	time.year = 0;
	
	rtc_update_display(BOTTOM,time.minutes);
	rtc_update_display(TOP,time.hours);
}

void rtc_set_time(uint8_t s, 
uint8_t m, 
uint8_t h,
uint16_t d,
uint16_t y) {
	
	cli();
	
	//time.seconds = s;
	time.minutes = m;
	time.hours = h;
	//time.days = d;
	//time.weeks = w;
	//time.year = y;
	
	rtc_update_display(BOTTOM,time.minutes);
	rtc_update_display(TOP,time.hours);
	
	sei();
}

void rtc_increment_hour(void) {
	if (time.hours < 59) {
		rtc_update_display(TOP,++time.hours);
	} else {
		time.hours = 0;
		rtc_update_display(TOP,time.hours);	
	}
	sei();
}

void rtc_decrement_hour(void) {
	if (time.hours > 0) {
		rtc_update_display(TOP,--time.hours);	
	} else {
		time.hours = 59;
		rtc_update_display(TOP,time.hours);	
	}
}

void rtc_increment_minute(void) {
	if (time.minutes < 59) {
		rtc_update_display(BOTTOM,++time.minutes);
	} else {
		time.minutes = 0;
		rtc_update_display(BOTTOM,time.minutes);
	}
}

void rtc_decrement_minute(void) {
	if (time.minutes > 0) {
		rtc_update_display(BOTTOM,--time.minutes);	
	} else {
		time.minutes = 59;
		rtc_update_display(BOTTOM,time.minutes);
	}
}

void rtc_set_time_mode(void) {
	RTC.CTRL = RTC_PRESCALER_OFF_gc;
	ht1632c_blink(1);
}

void rtc_time_mode(void) {
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;
	ht1632c_blink(0);
}

void rtc_update_display(uint8_t pos, uint8_t t) {
	
	char buffer[2];
	char temp;
	
	sprintf(buffer, "%d", t);
	
	//add zero
	if (t < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	//Clear old numbers
	ht1632c_fill_rect(0,pos,16,8,0);
	
	//Write new numbers
	ht1632c_draw_char(2,pos,buffer[0],1,1);
	ht1632c_draw_char(9,pos,buffer[1],1,1);
	
	ht1632c_refresh_screen();
}

void rtc_update_display_alt(void) {
	char buffer_hours[3];
	char buffer_minutes[3];
	char buffer[20];
	char temp;
	
	sprintf(buffer_hours, "%d", time.hours);
	
	//add zero

	if (time.hours < 10) {
		temp = buffer_hours[0];
		buffer_hours[0] = '0';
		buffer_hours[1] = temp;
	}
	
	sprintf(buffer_minutes, "%d", time.minutes);
	
	//add zero
	if (time.minutes < 10) {
		temp = buffer_minutes[0];
		buffer_minutes[0] = '0';
		buffer_minutes[1] = temp;
	}
	
	memset(buffer,0,5);
	strcat(buffer,"Time:");
	strcat(buffer, buffer_hours);
	strcat(buffer, ":");
	strcat(buffer,buffer_minutes);
	
	ht1632c_scroll_print(buffer,2,2);
}

ISR(RTC_OVF_vect) {
	if (++time.seconds == 60) {
		
		time.seconds = 0;
			
		if (++time.minutes == 60) {
			
			time.minutes = 0;
			//rtc_update_display(BOTTOM,time.minutes);
			
			if (++time.hours == 24) {
				time.weeks += (++time.days)/7;
				time.year += time.weeks/52;
				time.hours = 0;
				//rtc_update_display(TOP,time.hours);
			} else {
				//rtc_update_display(TOP,time.hours);
			}
			time.minutes = 0;
		} else {
			//rtc_update_display(BOTTOM,time.minutes);
		}
	}
}
