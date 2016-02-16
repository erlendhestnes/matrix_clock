/*
 * rtc.c
 *
 * Created: 10/26/2014 10:12:05 AM
 *  Author: Administrator
 */ 

#include "rtc.h"
#include "../ht1632c/ht1632c.h"
#include "../../modules/time_functions/time_functions.h"
#include "../../modules/display/display.h"

static volatile bool disp_time = true;

void rtc_setup(void) 
{
	//Disable power reduction for RTC
	PR.PRGEN &= ~0x40;
	
	alarm_status = ALARM_OFF;
	
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

void rtc_force_update(void) 
{
	RTC.CNT = 1920;
}

void rtc_update_seconds(uint16_t seconds)
{
	//cli();
	if (seconds < 60) {
		RTC.CNT = 32*seconds;
	} else {
		RTC.CNT = 1920;
	}
	//sei();
}

void rtc_show_hours(void) 
{
	rtc_update_display(TOP_HALF,env.time.hours);
	disp_time = false;
}

void rtc_show_minutes(void) 
{
	rtc_update_display(BOTTOM_HALF,env.time.minutes);
	disp_time = false;
}

void rtc_enable_time_render(void) 
{
	env.menu_id = 0;
	rtc_update_display(BOTTOM_HALF,env.time.minutes);
	rtc_update_display(TOP_HALF,env.time.hours);
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
	display_draw_char(2,pos,buffer[0],1,1);
	display_draw_char(9,pos,buffer[1],1,1);
}

ISR(RTC_OVF_vect) 
{
	//Variable to check system-runtime
	env.runtime++;
	
	if (++env.time.minutes >= 60) {
		
		env.time.minutes = 0;
		if (disp_time) {
			rtc_update_display(BOTTOM_HALF,env.time.minutes);
			display_refresh_screen();
		}
		if (++env.time.hours >= 24) {
			if (env.time.weekday++ >= Sunday) {
				env.time.weekday = Monday;
				if (env.time.week++ >= 53) {
					env.time.week = 1;
				}
			}
			if (env.time.day++ >= time_get_days_in_month(env.time.month,env.time.year)) {
				env.time.day = 1;
				if (env.time.month++ >= December) {
					env.time.month = January;
					env.time.year++;
					env.time.week = time_get_weeknumber(env.time.day,env.time.month,env.time.year);
				}
			}
			env.time.hours = 0;
			if (disp_time) {
				rtc_update_display(TOP_HALF,env.time.hours);
				display_refresh_screen();
			}
		} else {
			if (disp_time) {
				rtc_update_display(TOP_HALF,env.time.hours);
				display_refresh_screen();
			}
		}
		env.time.minutes = 0;
	} else {
		if (disp_time) {
			rtc_update_display(BOTTOM_HALF,env.time.minutes);
			display_refresh_screen();
		}
	}
	if (alarm_status == ALARM_ON) {
		if (env.time.hours == env.alarm.hours) {
			if (env.time.minutes == env.alarm.minutes)
			{
				alarm_status = ALARM_TRIGGERED;
			}
		}
	}
}