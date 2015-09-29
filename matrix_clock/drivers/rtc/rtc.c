/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"
#include "../ht1632c/ht1632c.h"

#include <stdlib.h>
#include <stdio.h>

//static volatile time_env_t env_variables.time;

static volatile bool disp_time = true;

#define TOP		9
#define BOTTOM  0

void rtc_setup(void) {
	CCP = CCP_IOREG_gc;
	CLK.RTCCTRL = CLK_RTCSRC_TOSC32_gc | CLK_RTCEN_bm;
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	
	//RTC.PER = 32768;
	//interrupt once every minute
	RTC.PER = 1920;
	RTC.INTCTRL = RTC_OVFINTLVL_MED_gc;
	RTC.CNT = 0;
	RTC.COMP = 0;
	RTC.CTRL = RTC_PRESCALER_DIV1024_gc;
}

void rtc_init_time(void) {
	ht1632c_clear_screen();
	
	env_var.time.seconds = 0;
	env_var.time.minutes = 0;
	env_var.time.hours = 0;
	env_var.time.weekday = 0;
	env_var.time.week = 0;
	env_var.time.year = 0;
	
	rtc_update_display(BOTTOM,env_var.time.minutes);
	rtc_update_display(TOP,env_var.time.hours);
	ht1632c_refresh_screen();
}

void rtc_disable_time_render(void) {
	disp_time = false;
}

void rtc_show_hours(void) {
	rtc_update_display(TOP,env_var.time.hours);
	disp_time = false;
}

void rtc_show_minutes(void) {
	rtc_update_display(BOTTOM,env_var.time.minutes);
	disp_time = false;
}

void rtc_enable_time_render(void) {
	env_var.menu_id = 0;
	rtc_update_display(BOTTOM,env_var.time.minutes);
	rtc_update_display(TOP,env_var.time.hours);
	disp_time = true;
}

void rtc_refresh_display(void) {
	//Maybe not needed...	
}

void rtc_set_time(uint8_t s,
uint8_t m,
uint8_t h,
uint16_t d,
uint16_t y) {

	env_var.time.seconds = s;
	env_var.time.minutes = m;
	env_var.time.hours = h;
	env_var.time.weekday = d;
	env_var.time.week = 0;
	env_var.time.month = 0;
	env_var.time.year = y;
	
	rtc_update_display(TOP,env_var.time.hours);
	rtc_update_display(BOTTOM,env_var.time.minutes);
	ht1632c_refresh_screen();

}

void rtc_increment_hour(void) {
	if (env_var.time.hours < 59) {
		rtc_update_display(5,++env_var.time.hours);
	} else {
		env_var.time.hours = 0;
		rtc_update_display(5,env_var.time.hours);	
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_hour(void) {
	if (env_var.time.hours > 0) {
		rtc_update_display(5,--env_var.time.hours);	
	} else {
		env_var.time.hours = 59;
		rtc_update_display(5,env_var.time.hours);	
	}
	ht1632c_refresh_screen();
}

void rtc_increment_minute(void) {
	if (env_var.time.minutes < 59) {
		rtc_update_display(5,++env_var.time.minutes);
	} else {
		env_var.time.minutes = 0;
		rtc_update_display(5,env_var.time.minutes);
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_minute(void) {
	if (env_var.time.minutes > 0) {
		rtc_update_display(5,--env_var.time.minutes);	
	} else {
		env_var.time.minutes = 59;
		rtc_update_display(5,env_var.time.minutes);
	}
	ht1632c_refresh_screen();
}

void rtc_increment_second(void) {
	if (env_var.time.seconds < 59) {
		rtc_update_display(5,++env_var.time.seconds);
	} else {
		env_var.time.seconds = 0;
		rtc_update_display(5,env_var.time.seconds);
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_second(void) {
	if (env_var.time.seconds > 0) {
		rtc_update_display(5,--env_var.time.seconds);
	} else {
		env_var.time.seconds = 59;
		rtc_update_display(5,env_var.time.seconds);
	}
	ht1632c_refresh_screen();
}

void rtc_increment_day(void) {
	if (env_var.time.weekday <= Sunday) {
		ht1632c_draw_three_letter_word(get_day_name(env_var.time.weekday++));
	} else {
		env_var.time.weekday = Monday;
		ht1632c_draw_three_letter_word(get_day_name(Monday));
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_day(void) {
	if (env_var.time.weekday > Monday) {
		ht1632c_draw_three_letter_word(get_day_name(env_var.time.weekday--));
	} else {
		env_var.time.weekday = Sunday;
		ht1632c_draw_three_letter_word(get_day_name(Sunday));
	}
	ht1632c_refresh_screen();
}

void rtc_increment_month(void) {
	if (env_var.time.month <= December) {
		ht1632c_draw_three_letter_word(get_month_name(env_var.time.month++));
	} else {
		env_var.time.month = January;
		ht1632c_draw_three_letter_word(get_month_name(January));
	}
	ht1632c_refresh_screen();
}

void rtc_decrement_year(void) {
	char *year;
	itoa_simple(year,env_var.time.year--);
	ht1632c_draw_four_letter_word(year);
	ht1632c_refresh_screen();
}

void rtc_increment_year(void) {
	char *year;
	itoa_simple(year,env_var.time.year++);
	ht1632c_draw_four_letter_word(year);
	ht1632c_refresh_screen();
}

void rtc_decrement_month(void) {
	if (env_var.time.month > January) {
		ht1632c_draw_three_letter_word(get_month_name(env_var.time.month--));
	} else {
		env_var.time.month = December;
		ht1632c_draw_three_letter_word(get_month_name(December));
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
	
	itoa_simple(buffer,t);
	
	//append zero
	if (t < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	//Clear old numbers
	if (pos == BOTTOM) {
		ht1632c_draw_filled_rect(0,0,16,8,0);
	} else {
		ht1632c_draw_filled_rect(0,7,16,9,0);
	}
	
	//Write new numbers
	ht1632c_draw_char(1,pos,buffer[0],1,1);
	ht1632c_draw_char(9,pos,buffer[1],1,1);
}

void rtc_update_display_alt(void) {
	char buffer_hours[3];
	char buffer_minutes[3];
	char buffer[20];
	char temp;
	
	sprintf(buffer_hours, "%d", env_var.time.hours);
	
	//add zero

	if (env_var.time.hours < 10) {
		temp = buffer_hours[0];
		buffer_hours[0] = '0';
		buffer_hours[1] = temp;
	}
	
	sprintf(buffer_minutes, "%d", env_var.time.minutes);
	
	//add zero
	if (env_var.time.minutes < 10) {
		temp = buffer_minutes[0];
		buffer_minutes[0] = '0';
		buffer_minutes[1] = temp;
	}
	
	memset(buffer,0,5);
	strcat(buffer,"env_variables.time:");
	strcat(buffer, buffer_hours);
	strcat(buffer, ":");
	strcat(buffer,buffer_minutes);
	
	//ht1632c_scroll_print(buffer,2,2);
}

void rtc_update_seconds(void) {
	
	//This needs to be changed
	static uint8_t x = 0;
	static uint8_t y = 0;
	
	ht1632c_draw_pixel(x,y,0);
	
	if (x < 15 && y == 0) {
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

const char* get_day_name(weekdays_t day)
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


const char* get_month_name(months_t month)
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

uint8_t get_month_number(char *month)
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

uint8_t get_days_in_month(months_t month, uint16_t year) 
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

ISR(RTC_OVF_vect) {
	if (++env_var.time.minutes == 60) {
		
		env_var.time.minutes = 0;
		if (disp_time) {
			rtc_update_display(BOTTOM,env_var.time.minutes);
			ht1632c_refresh_screen();
		}
		
		//Todo: implement DST - Daylight Saving Time
		
		if (++env_var.time.hours == 24) {
			//This might crash...
			if (env_var.time.weekday++ == Saturday) {
				env_var.time.weekday = Sunday;
				if (env_var.time.week++ == 52) {
					env_var.time.week = 1;
				}
			}
			if (env_var.time.day++ == get_days_in_month(env_var.time.month,env_var.time.year)) {
				env_var.time.day = 1;
				if (env_var.time.month++ == December) {
					env_var.time.month = January;
					env_var.time.year++;
				}
			}

			env_var.time.hours = 0;
			if (disp_time) {
				rtc_update_display(TOP,env_var.time.hours);
				ht1632c_refresh_screen();
			}
			} else {
			if (disp_time) {
				rtc_update_display(TOP,env_var.time.hours);
				ht1632c_refresh_screen();
			}
		}
		env_var.time.minutes = 0;
		} else {
		if (disp_time) {
			rtc_update_display(BOTTOM,env_var.time.minutes);
			ht1632c_refresh_screen();
		}
	}
}