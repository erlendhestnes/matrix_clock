/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"

time_t time;

bool disp_time = true;

#define TOP		9
#define BOTTOM  0

void rtc_setup(void) {
	CCP = CCP_IOREG_gc;
	CLK.RTCCTRL = CLK_RTCSRC_TOSC32_gc | CLK_RTCEN_bm;
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	
	//RTC.PER = 32768;
	RTC.PER = 3000;
	RTC.INTCTRL = RTC_OVFINTLVL_MED_gc;
	RTC.CNT = 0;
	RTC.COMP = 0;
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;
}

void rtc_init_time(void) {
	ht1632c_clear_screen();
	
	time.seconds = 0;
	time.minutes = 0;
	time.hours = 0;
	time.days = 0;
	time.weeks = 0;
	time.year = 0;
	
	rtc_update_display(BOTTOM,time.minutes);
	rtc_update_display(TOP,time.hours);
	ht1632c_refresh_screen();
}

void rtc_disable_time_render(void) {
	disp_time = false;
}

void rtc_enable_time_render(void) {
	rtc_update_display(BOTTOM,time.minutes);
	rtc_update_display(TOP,time.hours);
	disp_time = true;
}
	

void rtc_set_time(uint8_t s, 
uint8_t m, 
uint8_t h,
uint16_t d,
uint16_t y) {

	time.seconds = s;
	time.minutes = m;
	time.hours = h;
	time.days = d;
	time.year = y;
	
	rtc_update_display(TOP,time.hours);
	rtc_update_display(BOTTOM,time.minutes);
	ht1632c_refresh_screen();

}

void rtc_increment_hour(void) {
	if (time.hours < 59) {
		rtc_update_display(TOP,++time.hours);
	} else {
		time.hours = 0;
		rtc_update_display(TOP,time.hours);	
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_hour(void) {
	if (time.hours > 0) {
		rtc_update_display(TOP,--time.hours);	
	} else {
		time.hours = 59;
		rtc_update_display(TOP,time.hours);	
	}
	ht1632c_refresh_screen();
}

void rtc_increment_minute(void) {
	if (time.minutes < 59) {
		rtc_update_display(BOTTOM,++time.minutes);
	} else {
		time.minutes = 0;
		rtc_update_display(BOTTOM,time.minutes);
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_minute(void) {
	if (time.minutes > 0) {
		rtc_update_display(BOTTOM,--time.minutes);	
	} else {
		time.minutes = 59;
		rtc_update_display(BOTTOM,time.minutes);
	}
	ht1632c_refresh_screen();
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
	ht1632c_draw_filled_rect(0,pos,16,8,0);
	
	//Write new numbers
	ht1632c_draw_char(2,pos,buffer[0],1,1);
	ht1632c_draw_char(9,pos,buffer[1],1,1);
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

void rtc_update_seconds(void) {
	
	//This needs to be changed
	static uint8_t x = 0;
	static uint8_t y = 0;
	
	ht1632c_draw_pixel(x,y,0);
	
	if (x < 15 && y == 0)
	{
		x++;
	} else if (x == 15 && y < 15) {
		y++;
	} else if (y == 15 && x > 0) {
		x--;
	} else if (x == 0 && y > 0) {
		y--;
	}
	ht1632c_draw_pixel(x,y,1);
	
	ht1632c_refresh_screen();
}

ISR(RTC_OVF_vect) {
	//rtc_update_seconds();
	if (++time.seconds == 60) {
		
		time.seconds = 0;
			
		if (++time.minutes == 60) {
			
			time.minutes = 0;
			if (disp_time) { 
				rtc_update_display(BOTTOM,time.minutes);
				ht1632c_refresh_screen();
			}
			
			if (++time.hours == 24) {
				time.weeks += (++time.days)/7;
				time.year += time.weeks/52;
				time.hours = 0;
				if (disp_time) {
					rtc_update_display(TOP,time.hours);
					ht1632c_refresh_screen();	
				}
			} else {
				if (disp_time) {
					rtc_update_display(TOP,time.hours);
					ht1632c_refresh_screen();
				}
			}
			time.minutes = 0;
		} else {
			if (disp_time) {
				rtc_update_display(BOTTOM,time.minutes);
				ht1632c_refresh_screen();	
			}
		}
	}
}
