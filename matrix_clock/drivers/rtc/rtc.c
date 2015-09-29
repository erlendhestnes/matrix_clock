/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"
#include "../ht1632c/ht1632c.h"
#include "../../modules/time/time_functions.h"

static volatile bool disp_time = true;

#define TOP_HALF		9
#define BOTTOM_HALF		0

void rtc_setup(void) 
{
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

void rtc_show_hours(void) 
{
	rtc_update_display(TOP_HALF,env_var.time.hours);
	disp_time = false;
}

void rtc_show_minutes(void) 
{
	rtc_update_display(BOTTOM_HALF,env_var.time.minutes);
	disp_time = false;
}

void rtc_enable_time_render(void) 
{
	env_var.menu_id = 0;
	rtc_update_display(BOTTOM_HALF,env_var.time.minutes);
	rtc_update_display(TOP_HALF,env_var.time.hours);
	disp_time = true;
}

void rtc_disable_time_render(void)
{
	disp_time = false;
}

void rtc_update_display(uint8_t pos, uint8_t time) 
{	
	char buffer[2];
	char temp;
	
	itoa_simple(buffer,time);
	
	//append zero
	if (time < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	//Clear old numbers
	if (pos == BOTTOM_HALF) {
		display_draw_filled_rect(0,0,16,8,0);
	} else {
		display_draw_filled_rect(0,7,16,9,0);
	}
	
	//Write new numbers
	display_draw_char(1,pos,buffer[0],1,1);
	display_draw_char(9,pos,buffer[1],1,1);
}

ISR(RTC_OVF_vect) 
{
	if (++env_var.time.minutes == 60) {
		
		env_var.time.minutes = 0;
		if (disp_time) {
			rtc_update_display(BOTTOM_HALF,env_var.time.minutes);
			display_refresh_screen();
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
			if (env_var.time.day++ == time_get_days_in_month(env_var.time.month,env_var.time.year)) {
				env_var.time.day = 1;
				if (env_var.time.month++ == December) {
					env_var.time.month = January;
					env_var.time.year++;
				}
			}

			env_var.time.hours = 0;
			if (disp_time) {
				rtc_update_display(TOP_HALF,env_var.time.hours);
				display_refresh_screen();
			}
			} else {
				if (disp_time) {
					rtc_update_display(TOP_HALF,env_var.time.hours);
					display_refresh_screen();
				}
		}
		env_var.time.minutes = 0;
		} else {
			if (disp_time) {
				rtc_update_display(BOTTOM_HALF,env_var.time.minutes);
				display_refresh_screen();
			}
	}
}