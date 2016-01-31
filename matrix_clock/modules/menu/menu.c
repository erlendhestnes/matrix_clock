/*
 * menu.c
 *
 * Created: 5/1/2015 10:35:39 PM
 *  Author: Administrator
 */ 

#include "menu.h"
#include "../../drivers/adc/adc.h"
#include "../../drivers/rtc/rtc.h"
#include "../../drivers/port/port.h"
#include "../../drivers/ht1632c/ht1632c.h"
#include "../../drivers/sensors/si114x/Si114x_functions.h"
#include "../../drivers/sensors/si114x/slider_algorithm.h"
#include "../../drivers/eeprom/eeprom.h"
#include "../../drivers/esp8266/esp8266.h"

#include "../json/jsmn.h"
#include "../json/json_functions.h"
#include "../display/display.h"
#include "../time_functions/time_functions.h"
#include "../fatfs/ff.h"
#include "../fatfs/sound.h"

#include <ctype.h>

static volatile uint16_t counter = 0;
static volatile bool GET_request = false;

#define TOKEN_BUFFER_SIZE 30

void play_sound(void)
{
	FATFS FatFs;		// FatFs work area needed for each volume
	FIL Fil;			// File object needed for each open file
	BYTE Buff[512];		// Working buffer 1024
	UINT bw;
	
	if (f_open(&Fil, "newfilea2.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	// Create a file
		
		f_write(&Fil, "It works!\r\n", 11, &bw);	// Write data to the file
		
		f_close(&Fil);								// Close the file
	}
	
	
	f_mount(&FatFs, "", 0);
	
	BYTE res;
	res = f_open(&Fil, "rath3.wav", FA_READ);
	if (!res) {
		load_wav(&Fil, "**** WAV PLAYER ****", Buff, sizeof Buff);
		f_close(&Fil);
	}
}

void start_loading(void) 
{
	display_clear_screen();
	
	TCC0.CNT = 0;
	TCC0.PER = 3125;
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	
	TCC0.CCA = 3125;
	TCC0.INTCTRLB |= TC_CCAINTLVL_LO_gc;
	TCC0.CTRLB |= TC0_CCAEN_bm;
}

void stop_loading(void) 
{
	TCC0.CTRLA = TC_CLKSEL_OFF_gc;
	display_clear_screen();
}

esp8266_status_t get_internet_variables(bool get_time, bool get_temperature) 
{	
	char json[250];
	jsmn_parser p;
	jsmnerr_t r;
	jsmntok_t tokens[25];
	
	esp8266_status_t status;
	
	esp8266_on();
	
	status = esp8266_setup();
	if (status != ESP8266_SUCCESS) {
		stop_loading();
#ifdef SHOW_MANUAL
		display_print_scrolling_text("SETUP ERROR",false);
#endif
		return status;
	}
	
	status = esp8266_join_ap(env_var.wifi_ssid,env_var.wifi_pswd);
	if (status != ESP8266_SUCCESS) {
		stop_loading();
#ifdef SHOW_MANUAL
		display_print_scrolling_text("COULD NOT JOIN AP",false);
#endif
		return status;
	}
	status = esp8266_get_json(WEATHER_IP,WEATHER_ADDR,json, sizeof(json));
	if (status != ESP8266_SUCCESS) {
		stop_loading();
#ifdef SHOW_MANUAL
		display_print_scrolling_text("COULD NOT CONTACT SERVER",false);
#endif
		return status;
	}
	
	esp8266_off();
#ifdef DEBUG_ON
	puts(json);
#endif

	//Parse JSON
	jsmn_init(&p);
	r = jsmn_parse(&p,json,strlen(json),tokens,sizeof(tokens));
	if (r < 0) {
		stop_loading();
#ifdef SHOW_MANUAL
		display_print_scrolling_text("ERROR IN PARSING JSON",false);
		return ESP8266_ERROR;
#endif
	}

	char token_buffer[TOKEN_BUFFER_SIZE];
	if (get_time)
	{
		int year, month, day, hour, minute, second;
	
		json_get_token(tokens,json,token_buffer,TOKEN_BUFFER_SIZE,6);
		if (sscanf(token_buffer,"%d-%d-%dT%d:%d:%dZ",&year, &month, &day, &hour, &minute, &second) != 6) {
			stop_loading();
#ifdef SHOW_MANUAL
			display_print_scrolling_text("COULD NOT GET TIME",false);
#endif
			return ESP8266_ERROR;
		} else {
			env_var.time.year = year;
			env_var.time.month = month;
			env_var.time.day = day;
			env_var.time.hours = hour + env_var.time.timezone + env_var.time.DST;
			env_var.time.minutes = minute;
			env_var.time.seconds = second;
			env_var.time.weekday = time_get_weekday(day,month,year);
			
			if (env_var.time.hours >= 24) {
				env_var.time.hours -= 24;
				if (env_var.time.weekday++ >= Sunday) {
					env_var.time.weekday = Monday;
				}
				if (env_var.time.day++ >= time_get_days_in_month(env_var.time.month,env_var.time.year)) {
					env_var.time.day = 1;
					if (env_var.time.month++ >= December) {
						env_var.time.month = January;
						env_var.time.year++;
					}
				}
			}
			env_var.time.week = time_get_weeknumber(env_var.time.day,env_var.time.month,env_var.time.year);
		}
	}
	if (get_temperature)
	{
		json_get_token(tokens,json,token_buffer,TOKEN_BUFFER_SIZE,22);
#ifdef DEBUG_ON
		puts(token_buffer);
#endif
		if (strlen(token_buffer) <= 3) {
			strncpy(env_var.temperature,token_buffer,strlen(token_buffer));	
			json_get_token(tokens,json,token_buffer,TOKEN_BUFFER_SIZE,24);
#ifdef DEBUG_ON
			puts(token_buffer);
#endif
			strncpy(env_var.weather_info,token_buffer,strlen(token_buffer));
		} else {
			env_var.temperature[0] = '-';
			return ESP8266_ERROR;
		}
	}

	return ESP8266_SUCCESS;
}

void menu_draw_temperature_frame(void) 
{
	display_draw_small_char(1,10,'T',1,1);
	display_draw_small_char(5,10,'E',1,1);
	display_draw_small_char(9,10,'M',1,1);
	display_draw_small_char(13,10,'P',1,1);
	
	if (strlen(env_var.temperature) < 2) {
		display_draw_small_char(5,3,env_var.temperature[0],1,1);
		display_draw_small_char(9,3,'C',1,1);	
	} else {
		display_draw_small_char(3,3,env_var.temperature[0],1,1);
		display_draw_small_char(7,3,env_var.temperature[1],1,1);
		display_draw_small_char(11,3,'C',1,1);	
	}
}

void menu_draw_date_frame(void) 
{
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

void menu_draw_week_frame(void) 
{
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

void menu_draw_weekday_frame(void) 
{
	display_draw_small_char(1,10,'W',1,1);
	display_draw_small_char(5,10,'D',1,1);
	display_draw_small_char(9,10,'A',1,1);
	display_draw_small_char(13,10,'Y',1,1);
	
	char buffer[3];
	strncpy(buffer,time_get_day_name(env_var.time.weekday),3);
	
	display_draw_small_char(3,3,buffer[0],1,1);
	display_draw_small_char(7,3,buffer[1],1,1);
	display_draw_small_char(11,3,buffer[2],1,1);
}

void menu_draw_battery_frame(void) 
{	
	display_draw_small_char(1,10,'B',1,1);
	display_draw_small_char(5,10,'A',1,1);
	display_draw_small_char(9,10,'T',1,1);
	display_draw_small_char(13,10,'T',1,1);
	
	uint8_t battery = adc_get_battery_percentage();
	
	char buffer[5];
	itoa(battery,buffer,10);
	
	display_draw_small_char(3,3,buffer[0],1,1);
	display_draw_small_char(7,3,buffer[1],1,1);
	display_draw_small_char(11,3,'%',1,1);
	
}

void menu_draw_config_frame(void) 
{
	display_draw_four_letter_word("CONF");
}

void menu_set_env_variables(void) 
{
	EEPROM_EraseAll();
	
	strncpy(env_var.name,CLOCK_NAME,sizeof(env_var.name));
	env_var.id = CLOCK_ID;
	env_var.temperature[0] = '0';
	env_var.brightness = 0;
	env_var.menu_id = 0;
	strncpy(env_var.wifi_pswd,PASS, strlen(PASS));
	strncpy(env_var.wifi_ssid,SSID, strlen(SSID));
	
	env_var.time.timezone = 1;
	env_var.time.DST = 0;
	env_var.time.seconds = 0;
	env_var.time.minutes = 0;
	env_var.time.hours = 0;
	env_var.time.day = 0;
	env_var.time.month = January;
	env_var.time.year = 2016;
	env_var.time.weekday = time_get_weekday(env_var.time.day, env_var.time.month, env_var.time.year);
	env_var.time.week = time_get_weeknumber(env_var.time.day, env_var.time.month, env_var.time.year);;
	
	env_var.alarm.hours = 0;
	env_var.alarm.minutes = 0;
	
	env_var.runtime = 0;
}

menu_status_t menu_state_machine(SI114X_IRQ_SAMPLE *samples) 
{	
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
			case MENU_TIME:
				rtc_enable_time_render();
				break;
			case MENU_DATE:
				menu_draw_date_frame();
				break;
			case MENU_WEEKDAY:
				menu_draw_weekday_frame();
				break;
			case MENU_WEEK:
				menu_draw_week_frame();
				break;
			case MENU_TEMP:
				menu_draw_temperature_frame();
				break;
			case MENU_BATTERY:
				menu_draw_battery_frame();
				break;
			case MENU_CONFIG:
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

	if (samples->gesture == PAUSE) {
		if (env_var.menu_id == MENU_CONFIG) {
			display_fade_blink();
			display_slide_out_to_top();
#ifdef SHOW_MANUAL
			display_print_scrolling_text("CONFIG MENU. USE BUTTONS",false);
#endif
			display_draw_four_letter_word("<  >");
			display_slide_in_from_bottom();
			env_var.menu_id = 0;
			while(menu_configuration(samples) == MENU_SUCCESS);
			env_var.menu_id = 0;
			rtc_enable_time_render();
			display_slide_in_from_top();
			_delay_ms(1000);
		} else if (env_var.menu_id == MENU_TEMP) {
			display_fade_blink();			
			display_slide_out_to_top();
			esp8266_status_t status;
			start_loading();
			status = get_internet_variables(true,true);
			esp8266_off(); //guard
			stop_loading();
			
			if (status == ESP8266_SUCCESS) {
				char weather_info[50];
				strcpy(weather_info, "WEATHER FOR TRONDHEIM NORWAY: ");
				strcat(weather_info,env_var.temperature);
				strcat(weather_info,"C ");
				uint8_t i = 0;
				while(env_var.weather_info[i]) {
					env_var.weather_info[i] = toupper(env_var.weather_info[i]);
					i++;
				}
				strcat(weather_info,env_var.weather_info);
#ifdef SHOW_MANUAL
				display_print_scrolling_text(weather_info, false);
#endif
			} else {
#ifdef SHOW_MANUAL
				display_print_scrolling_text("COULD NOT GET TEMPERATURE.", false);
#endif
			}
			menu_draw_temperature_frame();
			display_slide_in_from_top();
		} 
	}
	return ESP8266_SUCCESS;
}

menu_status_t menu_configuration(SI114X_IRQ_SAMPLE *samples) 
{	
	btn_status = btn_check_press();
	
	//Menu ID select
	if (btn_status != NO_BTN) {
		if (btn_status == BTN1) {
			if (env_var.menu_id != NUMBER_OF_CONFIG_MENUS) {
				env_var.menu_id++;
			} else {
				env_var.menu_id = 0;
			}
		} else if (btn_status == BTN4) {
			if (env_var.menu_id != 0) {
				env_var.menu_id--;
			} else {
				env_var.menu_id = NUMBER_OF_CONFIG_MENUS;
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
	if (btn_status != NO_BTN) {
		switch(env_var.menu_id) {
			case CONFIG_BRIGHTNESS:
				display_draw_four_letter_word("LGHT");
				break;
			case CONFIG_TIME:
				display_draw_four_letter_word("TIME");
				break;
			case CONFIG_WIFI:
				display_draw_four_letter_word("WIFI");
				break;
			case CONFIG_ALARM:
				display_draw_four_letter_word("ALRM");
				break;
			case CONFIG_CALIBRATE:
				display_draw_four_letter_word("CALI");
				break;
			case CONFIG_INFO:
				display_draw_four_letter_word("INFO");
				break;
			case CONFIG_MUSIC:
				display_draw_four_letter_word("DEMO");
				break;
			case CONFIG_EXIT:
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
		switch(env_var.menu_id) {
			case CONFIG_BRIGHTNESS:
				display_slide_out_to_top();
				menu_configure_brightnesss();
				ht1632c_set_brightness(MAX_BRIGHTNESS);
				break;
			case CONFIG_TIME:
				display_slide_out_to_top();
				menu_set_time();
				break;
			case CONFIG_WIFI:
				display_slide_out_to_top();
				display_print_scrolling_text("CONNECT TO AP AND ENTER THE UPCOMMING IP ADDRESS",false);
				//Feature: Should draw wifi lines on display here.
				esp8266_on();
				if (esp8266_setup_webserver(false,true) == ESP8266_SUCCESS) {
					while(esp8266_configure_ssid_and_password() != ESP8266_TIMEOUT) {
						btn_status = btn_check_press();
						if (btn_status == BTN4)
						{
							break;
						}
					}
				} else {
#ifdef SHOW_MANUAL
					display_print_scrolling_text("SOMETHING WENT WRONG",false);
#endif					
				}
				esp8266_off();
				break;
			case CONFIG_ALARM:
				display_slide_out_to_top();
				menu_set_alarm();
				break;
			case CONFIG_CALIBRATE:
				display_slide_out_to_top();
				si114x_baseline_calibration(samples);
				break;
			case CONFIG_INFO:
				display_slide_out_to_top();
				display_print_scrolling_text("MADE BY: ERLEND HESTNES",false);
				break;
			case CONFIG_MUSIC:
				play_sound();
				break;
			case CONFIG_EXIT:
				display_slide_out_to_bottom();
				return MENU_CLOSE;
			default:
				break;
		}
		env_var.menu_id = 0;
		display_slide_out_to_bottom();
		display_draw_four_letter_word("<  >");
		display_slide_in_from_top();
	}	
	return MENU_SUCCESS;
}

void menu_configure_brightnesss(void) 
{	
	bool quit = false;
	
	ht1632c_set_brightness(env_var.brightness);
	display_draw_filled_rect(0,7,env_var.brightness+1,2,1);
	display_refresh_screen();
	
	while(!quit) {
		btn_status = btn_check_press();

		switch(btn_status) {
			case BTN4:
				if (env_var.brightness < 15) {
					display_draw_filled_rect(0,7,env_var.brightness+1,2,0);
					ht1632c_set_brightness(++env_var.brightness);
					display_draw_filled_rect(0,7,env_var.brightness+1,2,1);
					display_refresh_screen();
				}
				_delay_ms(100);
				break;
			case BTN1:
				if (env_var.brightness >= 0) {
					display_draw_filled_rect(0,7,env_var.brightness+1,2,0);
					ht1632c_set_brightness(--env_var.brightness);
					display_draw_filled_rect(0,7,env_var.brightness+1,2,1);
					display_refresh_screen();
				}
				_delay_ms(100);
				break;
			case BTN3:
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("BRIGHTNESS IS SET",false);
#endif
				quit = true;
				EEPROM_WriteEnv();
				break;
			case BTN2:
#ifdef SHOW_MANUAL
				display_print_scrolling_text("EXIT",false);
#endif
				return;
			default:
				_delay_ms(100);
				btn_status = NO_BTN;
				break;
		}
	}
}

uint8_t menu_set_time(void) 
{	
	bool next = false;
	char buffer[3];
	
	display_draw_three_letter_word("GMT");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	itoa_simple(buffer,env_var.time.timezone);
	display_draw_small_char(3,10,'G',1,1);
	display_draw_small_char(7,10,'M',1,1);
	display_draw_small_char(11,10,'T',1,1);
	if (env_var.time.timezone > 0) {
		display_draw_small_char(5,3,'+',1,1);
		} else if (env_var.time.timezone < 0) {
		display_draw_small_char(5,3,'-',1,1);
	}
	display_draw_small_char(9,3,buffer[0],1,1);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				if (env_var.time.timezone < 9) {
					display_clear_screen();
					env_var.time.timezone++;
					itoa_simple(buffer,env_var.time.timezone);
					display_draw_small_char(3,10,'G',1,1);
					display_draw_small_char(7,10,'M',1,1);
					display_draw_small_char(11,10,'T',1,1);
					if (env_var.time.timezone > 0) {
						display_draw_small_char(5,3,'+',1,1);
						display_draw_small_char(9,3,buffer[0],1,1);
					} else if (env_var.time.timezone < 0) {
						display_draw_small_char(5,3,'-',1,1);
						display_draw_small_char(9,3,buffer[1],1,1);
					} else {
						display_draw_small_char(8,3,'0',1,1);
					}
					display_refresh_screen();
				}
				_delay_ms(250);
				break;
			case BTN1:
				if (env_var.time.timezone > -9) {
					display_clear_screen();
					env_var.time.timezone--;
					itoa_simple(buffer,env_var.time.timezone);
					display_draw_small_char(3,10,'G',1,1);
					display_draw_small_char(7,10,'M',1,1);
					display_draw_small_char(11,10,'T',1,1);
					if (env_var.time.timezone > 0) {
						display_draw_small_char(5,3,'+',1,1);
						display_draw_small_char(9,3,buffer[0],1,1);
					} else if (env_var.time.timezone < 0) {
						display_draw_small_char(5,3,'-',1,1);
						display_draw_small_char(9,3,buffer[1],1,1);
					} else {
						display_draw_small_char(8,3,'0',1,1);
					}
					display_refresh_screen();
				}
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				//ht1632c_blink(false);
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	display_slide_out_to_left();
	display_draw_three_letter_word("DST");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	if (env_var.time.DST) {
		display_draw_four_letter_word(" ON ");
	} else {
		display_draw_three_letter_word("OFF");
	}
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				env_var.time.DST = 1;
				display_draw_four_letter_word(" ON ");
				display_refresh_screen();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				env_var.time.DST = 0;
				display_draw_three_letter_word("OFF");
				display_refresh_screen();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				//ht1632c_blink(false);
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	display_slide_out_to_left();
	display_draw_three_letter_word("HRS");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.time.hours);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_draw_and_increment_hour();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				display_draw_and_decrement_hour();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("MIN");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.time.minutes);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set minutes
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_draw_and_increment_minute();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				display_draw_and_decrement_minute();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("SEC");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.time.seconds);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set seconds
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_draw_and_increment_second();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				display_draw_and_decrement_second();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_four_letter_word("WDAY");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	display_draw_three_letter_word(time_get_day_name(env_var.time.weekday));
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set day
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_draw_and_increment_day();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				display_draw_and_decrement_day();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_four_letter_word("MNTH");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	display_draw_three_letter_word(time_get_month_name(env_var.time.month));
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set month
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_draw_and_increment_month();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				display_draw_and_decrement_month();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_four_letter_word("YEAR");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	char *year = NULL;
	itoa_simple(year,env_var.time.year);
	display_draw_four_letter_word(year);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set year
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_draw_and_increment_year();
				_delay_ms(250);
				break;
			case BTN1:
				display_clear_screen();
				display_draw_and_decrement_year();
				_delay_ms(250);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	EEPROM_WriteEnv();
	display_slide_out_to_bottom();
	
#ifdef SHOW_MANUAL
	display_print_scrolling_text("TIME AND DATE IS SET",false);
#endif
	
	return 1;
}

uint8_t menu_set_alarm(void) 
{
	bool next = false;
	
	display_draw_three_letter_word("HRS");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.alarm.hours);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_alarm_increment_hour();
				_delay_ms(150);
				break;
			case BTN1:
				display_clear_screen();
				display_alarm_decrement_hour();
				_delay_ms(150);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	display_slide_out_to_left();
	display_draw_three_letter_word("MIN");
	display_slide_in_from_right();
	_delay_ms(1000);
	display_slide_out_to_left();
	rtc_update_display(5,env_var.alarm.minutes);
	display_slide_in_from_right();
	//ht1632c_blink(true);
	
	//Set minutes
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN4:
				display_clear_screen();
				display_alarm_increment_minute();
				_delay_ms(150);
				break;
			case BTN1:
				display_clear_screen();
				display_alarm_decrement_minute();
				_delay_ms(150);
				break;
			case BTN3:
				next = true;
				_delay_ms(250);
				break;
			case BTN2:
				//ht1632c_blink(false);
				display_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				display_print_scrolling_text("CANCELLED",false);
#endif
				return 0;
			default:
				btn_status = NO_BTN;
				break;
		}
	}
	next = false;
	
	//ht1632c_blink(false);
	EEPROM_WriteEnv();
	display_slide_out_to_bottom();
	
#ifdef SHOW_MANUAL
	display_print_scrolling_text("ALARM IS SET",false);
#endif

	return 1;
}

ISR(TCC0_CCA_vect) 
{
	display_show_loading_square();
}