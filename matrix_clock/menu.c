/*
 * menu.c
 *
 * Created: 5/1/2015 10:35:39 PM
 *  Author: Administrator
 */ 

#include "menu.h"
#include "drivers/rtc/rtc.h"
#include "drivers/port/port.h"
#include "drivers/ht1632c/ht1632c.h"

void menu_state_machine(void) {
	if (btn_status != NONE)
	{
		switch(btn_status) {
			case BTN1:
			rtc_increment_minute();
			_delay_ms(250);
			break;
			case BTN2:
			rtc_decrement_minute();
			_delay_ms(250);
			break;
			case BTN3:
			rtc_set_time_mode();
			//rtc_increment_hour();
			_delay_ms(250);
			break;
			case BTN4:
			rtc_time_mode();
			//rtc_decrement_hour();
			_delay_ms(250);
			break;
		}
		btn_status = NONE;
	}
}

