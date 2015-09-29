/*
 * menu.c
 *
 * Created: 5/1/2015 10:35:39 PM
 *  Author: Administrator
 */ 

#include "menu.h"
#include "../../drivers/rtc/rtc.h"
#include "../../drivers/port/port.h"
#include "../../drivers/ht1632c/ht1632c.h"
#include "../../drivers/esp8266/esp8266.h"
#include "../../drivers/sensors/si114x/Si114x_functions.h"
#include "../../drivers/sensors/si114x/slider_algorithm.h"
#include "../../json/jsmn.h"

static volatile uint16_t counter = 0;

void start_loading(void) {
	display_clear_screen();
	
	TCC0.CNT = 0;
	TCC0.PER = 3125;
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	
	TCC0.CCA = 3125;
	TCC0.INTCTRLB |= TC_CCAINTLVL_LO_gc;
	TCC0.CTRLB |= TC0_CCAEN_bm;
}

void stop_loading(void) {
	TCC0.CTRLA = TC_CLKSEL_OFF_gc;
	display_clear_screen();
}

esp8266_status_t get_internet_variables(bool get_time, bool get_temperature) {
	
	char json[250];
	jsmn_parser p;
	jsmnerr_t r;
	jsmntok_t tokens[25];
	
	esp8266_status_t status;
	
	esp8266_on();
	
	status = esp8266_setup();
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	status = esp8266_join_ap(SSID,PASS);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	status = esp8266_get_json(WEATHER_IP,WEATHER_ADDR,json, 202);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	esp8266_off();
#ifdef DEBUG_ON
	puts(json);
#endif

	//Parse JSON
	jsmn_init(&p);
	r = jsmn_parse(&p,json,strlen(json),tokens,25);

	char token_buffer[20];
	if (get_time)
	{
		int year, month, day, hour, minute, second;
	
		json_get_token(tokens,json,token_buffer,6);
		if (sscanf(token_buffer,"%d-%d-%dT%d:%d:%dZ",&year, &month, &day, &hour, &minute, &second) != 6) {
#ifdef DEBUG_ON
			puts("Failed to parse time for some reason...");
#endif
		} else {
#ifdef DEBUG_ON
			puts("Parsed time successfully!");
#endif
			env_var.time.year = year;
			env_var.time.month = month;
			env_var.time.day = day;
			env_var.time.hours = hour + 2;
			env_var.time.minutes = minute;
			env_var.time.seconds = second;
			env_var.time.weekday = time_get_day_of_week(day+1,month,year);
#ifdef DEBUG_ON
			printf("Weekday: %d \r\n",env_var.time.weekday);
#endif
		}
		//printf("%d , %d , %d , %d , %d , %d \r\n",year,month,day,hour,minute,second);
	}
	if (get_temperature)
	{
		json_get_token(tokens,json,token_buffer,22);
		strncpy(env_var.temperature,token_buffer,strlen(token_buffer));
			
		json_get_token(tokens,json,token_buffer,24);
		strncpy(env_var.weather_info,token_buffer,strlen(token_buffer));
	}

	return ESP8266_SUCCESS;
}

void menu_draw_temperature_frame(void) {
	display_draw_small_char(1,10,'T',1,1);
	display_draw_small_char(5,10,'E',1,1);
	display_draw_small_char(9,10,'M',1,1);
	display_draw_small_char(13,10,'P',1,1);
	
	display_draw_small_char(3,3,env_var.temperature[0],1,1);
	display_draw_small_char(7,3,env_var.temperature[1],1,1);
	display_draw_small_char(11,3,'C',1,1);
}

void menu_draw_date_frame(void) {
	display_draw_small_char(1,10,'D',1,1);
	display_draw_small_char(5,10,'A',1,1);
	display_draw_small_char(9,10,'T',1,1);
	display_draw_small_char(13,10,'E',1,1);
	
	char buffer[2];
	char temp;
	
	itoa_simple(buffer,env_var.time.day);
	
	//append zero
	if (env_var.time.day < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	display_draw_small_char(1,3,buffer[0],1,1);
	display_draw_small_char(4,3,buffer[1],1,1);
	display_draw_small_char(7,3,'.',1,1);
	
	itoa_simple(buffer,env_var.time.month);
	
	//append zero
	if (env_var.time.month < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	display_draw_small_char(10,3,buffer[0],1,1);
	display_draw_small_char(13,3,buffer[1],1,1);
}

void menu_draw_week_frame(void) {
	display_draw_small_char(1,10,'W',1,1);
	display_draw_small_char(5,10,'E',1,1);
	display_draw_small_char(9,10,'E',1,1);
	display_draw_small_char(13,10,'K',1,1);
	
	char buffer[2];
	char temp;
	
	itoa_simple(buffer,env_var.time.week);
	
	//append zero
	if (env_var.time.week < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	display_draw_small_char(5,3,buffer[0],1,1);
	display_draw_small_char(9,3,buffer[1],1,1);
}

void menu_draw_config_frame(void) {
	display_draw_three_letter_word("CFG");
}

void menu_set_env_variables(void) 
{
	EEPROM_EraseAll();
	
	strncpy(env_var.name,CLOCK_NAME,sizeof(env_var.name));
	env_var.id = CLOCK_ID;
	env_var.temperature[0] = '0';
	env_var.brightness = 1;
	env_var.menu_id = 0;
	strncpy(env_var.wifi_pswd,PASS,sizeof(env_var.wifi_pswd));
	strncpy(env_var.wifi_ssid,SSID,sizeof(env_var.wifi_ssid));
	
	env_var.time.seconds = 30;
	env_var.time.minutes = 59;
	env_var.time.hours = 23;
	env_var.time.day = 31;
	env_var.time.weekday = Sunday;
	env_var.time.week = 52;
	env_var.time.month = December;
	env_var.time.year = 2015;
	
	env_var.alarm.hours = 0;
	env_var.alarm.minutes = 0;
}

menu_status_t menu_state_machine(SI114X_IRQ_SAMPLE *samples) {
	
	//Menu ID select
	if (samples->gesture != NO_GESTURE) {
		if (samples->gesture == LEFT_SWIPE) {
			if (env_var.menu_id != NUMBER_OF_MENUS) {
				env_var.menu_id++;
			} else {
				env_var.menu_id = 0;
			}
		} else if (samples->gesture == RIGHT_SWIPE) {
			if (env_var.menu_id != 0) {
				env_var.menu_id--;
			} else {
				env_var.menu_id = NUMBER_OF_MENUS;
			}
		}
	}
	
	if (samples->gesture == LEFT_SWIPE) {
		rtc_disable_time_render();
		display_slide_out_to_right();
	} else if (samples->gesture == RIGHT_SWIPE) {
		rtc_disable_time_render();
		display_slide_out_to_left();
	}
	
	//Switch between menus
	if (samples->gesture != NO_GESTURE) {
		switch(env_var.menu_id) {
			case 0:
				rtc_enable_time_render();
				break;
			case 1:
				menu_draw_date_frame();
				break;
			case 2:
				menu_draw_week_frame();
				break;
			case 3:
				menu_draw_temperature_frame();
				break;
			case 4:
				menu_draw_config_frame();
				break;
			default:
				break;
		}	
	}
	
	if (samples->gesture == LEFT_SWIPE) {
		display_slide_in_from_left();
	} else if (samples->gesture == RIGHT_SWIPE) {
		display_slide_in_from_right();
	}
	
	btn_status = NONE;

	if (samples->gesture == PAUSE) {
		
		if (env_var.menu_id == 4)
		{
			display_fade_blink();
			
			display_slide_out_to_top();
#ifdef SHOW_MANUAL
			display_print_scrolling_text("CONFIG MENU, USE BUTTONS",false);
#endif
			display_draw_small_char(2,7,'<',1,1);
			display_draw_small_char(13,7,'>',1,1);
			display_slide_in_from_bottom();
			while(menu_configuration() == MENU_SUCCESS);
			
			env_var.menu_id = 0;
			rtc_enable_time_render();
			display_slide_in_from_top();
			
			_delay_ms(1000);
		} else if (env_var.menu_id == 3) {
			display_fade_blink();
			
			display_slide_out_to_top();
#ifdef SHOW_MANUAL
			display_print_scrolling_text("GETTING TEMPERATURE...",false);
#endif
			esp8266_status_t status;
			start_loading();
			status = get_internet_variables(true,true);
			stop_loading();
			
#ifdef SHOW_MANUAL
			if (status == ESP8266_SUCCESS) {
				char weather_info[50];
				strcpy(weather_info, "WEATHER FOR TRONDHEIM NORWAY: ");
				strcat(weather_info,env_var.temperature);
				strcat(weather_info,"C, ");
				strcat(weather_info,toupper(env_var.weather_info));
				display_print_scrolling_text(weather_info, false);
			} else {
				display_print_scrolling_text("COULD NOT GET TEMPERATURE...", false);
			}
#endif
			menu_draw_temperature_frame();
			display_slide_in_from_top();
			
		}
	}
}

menu_status_t menu_configuration(void) {
	
	btn_status = btn_check_press();
	
	//Menu ID select
	if (btn_status != NONE) {
		if (btn_status == BTN1) {
			if (env_var.menu_id != NUMBER_OF_MENUS) {
				env_var.menu_id++;
			} else {
				env_var.menu_id = 0;
			}
		} else if (btn_status == BTN4) {
			if (env_var.menu_id != 0) {
				env_var.menu_id--;
			} else {
				env_var.menu_id = NUMBER_OF_MENUS;
			}
		}
	}
	
	if (btn_status == BTN1) {
		rtc_disable_time_render();
		display_slide_out_to_right();
	} else if (btn_status == BTN4) {
		rtc_disable_time_render();
		display_slide_out_to_left();
	} else if (btn_status == BTN3) {
		display_fade_blink();
	}
	
	//Switch between menus
	if (btn_status != NONE) {
		switch(env_var.menu_id) {
			case 0:
				display_draw_four_letter_word("LGHT");
				break;
			case 1:
				display_draw_four_letter_word("TIME");
				break;
			case 2:
				display_draw_four_letter_word("WIFI");
				break;
			case 3:
				display_draw_four_letter_word("ALRM");
				break;
			case 4:
				display_draw_four_letter_word("EXIT");
				break;
			default:
				break;
		}	
	}
	
	if (btn_status == BTN1) {
		display_slide_in_from_left();
	} else if (btn_status == BTN4) {
		display_slide_in_from_right();
	}
	
	//Menu actions
	if (btn_status == BTN3) {
		//Used for wifi
		esp8266_status_t status;
		switch(env_var.menu_id) {
			case 0:
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("USE BUTTONS TO ADJUST BRIGHTNESS",false);
#endif
				menu_configure_brightnesss();
				ht1632c_set_brightness(MAX_BRIGHTNESS);
				break;
			case 1:
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("USE BUTTONS TO SET THE TIME",false);
				_delay_ms(500);
#endif
				menu_set_time();
				break;
			case 2:
				esp8266_on();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("STARTING TELNET SERVICE",false);
#endif
				status = esp8266_setup_webserver(true, false);
				if (status == ESP8266_SUCCESS) {
					//EDIT: Removed this, takes much RAM
					//esp8266_telnet_server();
				} else {
#ifdef SHOW_MANUAL
					display_print_scrolling_text("ERROR: COULD NOT START TELNET SERVICE",false);
#endif
				}
				break;
			case 3:
#ifdef SHOW_MANUAL
				display_print_scrolling_text("USE BUTTONS TO SET THE ALARM",false);
				_delay_ms(500);
#endif
				display_slide_out_to_bottom();
				menu_set_alarm();
			case 4:
				display_slide_out_to_bottom();
				return MENU_CLOSE;
			default:
				break;
		}
		display_slide_out_to_bottom();
		display_draw_four_letter_word("<  >");
		display_slide_in_from_top();
	}	
	return MENU_SUCCESS;
}

void menu_configure_brightnesss(void) {
	
	bool quit = false;
	
	ht1632c_set_brightness(env_var.brightness);
	display_draw_filled_rect(7,0,2,env_var.brightness,1);
	display_refresh_screen();
	
	while(!quit) {
		btn_status = btn_check_press();

		switch(btn_status) {
			case BTN1:
				if (env_var.brightness < 15) {
					display_draw_filled_rect(7,0,2,env_var.brightness,0);
					ht1632c_set_brightness(++env_var.brightness);
					//printf("Brightness: %d \n",env_var.brightness);
					display_draw_filled_rect(7,0,2,env_var.brightness,1);
					display_refresh_screen();
				}
				_delay_ms(100);
				break;
			case BTN2:
				if (env_var.brightness > 1) {
					display_draw_filled_rect(7,0,2,env_var.brightness,0);
					ht1632c_set_brightness(--env_var.brightness);
					//printf("Brightness: %d \n",env_var.brightness);
					display_draw_filled_rect(7,0,2,env_var.brightness,1);
					display_refresh_screen();
				}
				_delay_ms(100);
				break;
			case BTN4:
				display_clear_screen();
				quit = true;
				EEPROM_WriteEnv();
				break;
			default:
				_delay_ms(100);
				btn_status = NONE;
				break;
		}
	}
}

uint8_t menu_set_time(void) {
	
	bool quit = false;
	bool next = false;
	
	display_draw_three_letter_word("HRS");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.time.hours);
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_draw_and_increment_hour();
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_draw_and_decrement_hour();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("MIN");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.time.minutes);
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set minutes
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_draw_and_increment_minute();
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_draw_and_decrement_minute();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("SEC");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.time.seconds);
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set seconds
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_draw_and_increment_second();
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_draw_and_decrement_second();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("DAY");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	display_draw_three_letter_word(time_get_day_name(env_var.time.weekday));
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set day
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_draw_and_increment_day();
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_draw_and_decrement_day();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_four_letter_word("MNTH");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	display_draw_three_letter_word(time_get_month_name(env_var.time.month));
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set month
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_draw_and_increment_month();
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_draw_and_decrement_month();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_four_letter_word("YEAR");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	char *year;
	itoa_simple(year,env_var.time.year);
	display_draw_four_letter_word(year);
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set year
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_draw_and_increment_year();
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_draw_and_decrement_year();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	
	EEPROM_WriteEnv();
	
#ifdef SHOW_MANUAL
	display_print_scrolling_text("TIME AND DATE IS NOW UPDATED",false);
#endif
	
	return 1;
}

uint8_t menu_set_alarm(void) 
{
	bool quit = false;
	bool next = false;
	
	display_draw_three_letter_word("HRS");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	display_alarm_increment_hour(env_var.alarm.hours);
	display_alarm_increment_minute(env_var.alarm.minutes);
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_alarm_increment_hour(env_var.alarm.hours++);
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_alarm_decrement_hour(env_var.alarm.hours--);
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("MIN");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.alarm.minutes);
	display_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set minutes
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				display_clear_screen();
				display_alarm_increment_minute(env_var.alarm.minutes++);
				_delay_ms(250);
				break;
			case BTN2:
				display_clear_screen();
				display_alarm_decrement_minute(env_var.alarm.minutes--);
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN4:
				ht1632c_blink(false);
				return 0;
			default:
				btn_status = NONE;
				break;
		}
	}
	next = false;
	
	ht1632c_blink(false);
	
	EEPROM_WriteEnv();
	
#ifdef SHOW_MANUAL
	display_print_scrolling_text("ALARM IS NOW SET",false);
#endif

	return 1;
}

ISR(TCC0_CCA_vect) {
	display_show_loading_square();
}