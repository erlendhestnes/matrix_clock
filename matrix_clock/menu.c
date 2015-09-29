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
#include "drivers/esp8266/esp8266.h"
#include "drivers/sensors/si114x/Si114x_functions.h"
#include "drivers/sensors/si114x/slider_algorithm.h"
#include "json/jsmn.h"

static volatile uint16_t counter = 0;

void get_json_token(jsmntok_t *tokens, char *js, char *buffer, uint8_t i) {
	int len;
	jsmntok_t key;

	key = tokens[i];
	len = key.end - key.start;
	char keyString[ len+1 ];
	memcpy( keyString, &js[ key.start ], len );
	keyString[ len ] = '\0';
	
	strcpy(buffer,keyString);
}

esp8266_status_t get_week(void) {
	char *json;
	json = (char *) calloc(125, sizeof(char));
	
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
	
	status = esp8266_get_json(TIME_IP,TIME_ADDR,json, 125);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	esp8266_off();
	
	//printf("got data: %s \r\n",json);
	puts(json);
	jsmn_init(&p);
	r = jsmn_parse(&p,json,strlen(json),tokens,25);
	
	char *buffer;
	buffer = (char *) calloc(20, sizeof(char));

	get_json_token(tokens,json,buffer,2);
	env_var.time.week = atoi(buffer);
	
	/*
	char *buffer;
	buffer = (char *) calloc(20, sizeof(char));

	get_token(tokens,json,buffer,22);
	strncpy(env_var.temperature,buffer,strlen(buffer));
	
	get_token(tokens,json,buffer,24);
	strncpy(env_var.weather_info,buffer,strlen(buffer));
	*/
	
	free(buffer);
	free(json);
	
	return ESP8266_SUCCESS;
}

void start_loading(void) {
	ht1632c_clear_screen();
	
	TCC0.CNT = 0;
	TCC0.PER = 3125;
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	
	TCC0.CCA = 3125;
	TCC0.INTCTRLB |= TC_CCAINTLVL_LO_gc;
	TCC0.CTRLB |= TC0_CCAEN_bm;
}

void stop_loading(void) {
	TCC0.CTRLA = TC_CLKSEL_OFF_gc;
	ht1632c_clear_screen();
}

int dayofweek(int d, int m, int y)
{
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	y -= m < 3;
	return ( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

esp8266_status_t get_temperature(void) {
	
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
	int year, month, day, hour, minute, second;
	
	get_json_token(tokens,json,token_buffer,6);
	if (sscanf(token_buffer,"%d-%d-%dT%d:%d:%dZ",&year, &month, &day, &hour, &minute, &second) != 6) {
#ifdef DEBUG_ON
		puts("Failed to parse time");
#endif
	} else {
#ifdef DEBUG_ON
		puts("Parsed time");
#endif
		env_var.time.year = year;
		env_var.time.month = month;
		env_var.time.day = day;
		env_var.time.hours = hour + 2;
		env_var.time.minutes = minute;
		env_var.time.seconds = second;
		env_var.time.weekday = dayofweek(day+1,month,year);
		printf("Weekday: %d \r\n",env_var.time.weekday);
	}
	//printf("%d , %d , %d , %d , %d , %d \r\n",year,month,day,hour,minute,second);

	get_json_token(tokens,json,token_buffer,22);
	strncpy(env_var.temperature,token_buffer,strlen(token_buffer));
	
	get_json_token(tokens,json,token_buffer,24);
	strncpy(env_var.weather_info,token_buffer,strlen(token_buffer));
	
	return ESP8266_SUCCESS;
}

void menu_draw_temperature_frame(void) {
	ht1632c_draw_char_small(1,10,'T',1,1);
	ht1632c_draw_char_small(5,10,'E',1,1);
	ht1632c_draw_char_small(9,10,'M',1,1);
	ht1632c_draw_char_small(13,10,'P',1,1);
	
	ht1632c_draw_char_small(3,3,env_var.temperature[0],1,1);
	ht1632c_draw_char_small(7,3,env_var.temperature[1],1,1);
	ht1632c_draw_char_small(11,3,'C',1,1);
}

void menu_draw_date_frame(void) {
	ht1632c_draw_char_small(1,10,'D',1,1);
	ht1632c_draw_char_small(5,10,'A',1,1);
	ht1632c_draw_char_small(9,10,'T',1,1);
	ht1632c_draw_char_small(13,10,'E',1,1);
	
	char buffer[2];
	char temp;
	
	itoa_simple(buffer,env_var.time.day);
	
	//append zero
	if (env_var.time.day < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	ht1632c_draw_char_small(1,3,buffer[0],1,1);
	ht1632c_draw_char_small(4,3,buffer[1],1,1);
	ht1632c_draw_char_small(7,3,'.',1,1);
	
	itoa_simple(buffer,env_var.time.month);
	
	//append zero
	if (env_var.time.month < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	ht1632c_draw_char_small(10,3,buffer[0],1,1);
	ht1632c_draw_char_small(13,3,buffer[1],1,1);
}

void menu_draw_week_frame(void) {
	ht1632c_draw_char_small(1,10,'W',1,1);
	ht1632c_draw_char_small(5,10,'E',1,1);
	ht1632c_draw_char_small(9,10,'E',1,1);
	ht1632c_draw_char_small(13,10,'K',1,1);
	
	char buffer[2];
	char temp;
	
	itoa_simple(buffer,env_var.time.week);
	
	//append zero
	if (env_var.time.week < 10) {
		temp = buffer[0];
		buffer[0] = '0';
		buffer[1] = temp;
	}
	
	ht1632c_draw_char_small(5,3,buffer[0],1,1);
	ht1632c_draw_char_small(9,3,buffer[1],1,1);
}

void menu_state_machine(SI114X_IRQ_SAMPLE *samples) {
	
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
		ht1632c_slide_out_to_right();
	} else if (samples->gesture == RIGHT_SWIPE) {
		rtc_disable_time_render();
		ht1632c_slide_out_to_left();
	} else if (samples->gesture == PAUSE) {
		ht1632c_fade_blink();
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
				ht1632c_draw_three_letter_word("CFG");
				break;
			default:
				break;
		}	
	}
	
	if (samples->gesture == LEFT_SWIPE) {
		ht1632c_slide_in_from_left();
	} else if (samples->gesture == RIGHT_SWIPE) {
		ht1632c_slide_in_from_right();
	}
	
	btn_status = NONE;

	if (samples->gesture == PAUSE) {
		if (env_var.menu_id == 4)
		{
			ht1632c_slide_out_to_top();
#ifdef SHOW_MANUAL
			ht1632c_scroll_print("CONFIG MENU, USE BUTTONS",false);
#endif
			ht1632c_draw_char_small(2,7,'<',1,1);
			ht1632c_draw_char_small(13,7,'>',1,1);
			ht1632c_slide_in_from_bottom();
			while(menu_configuration());
			
			env_var.menu_id = 0;
			rtc_enable_time_render();
			ht1632c_slide_in_from_top();
			
			_delay_ms(1000);
		} else if (env_var.menu_id == 3) {
			ht1632c_slide_out_to_top();
#ifdef SHOW_MANUAL
			ht1632c_scroll_print("GETTING TEMPERATURE...",false);
#endif
			esp8266_status_t status;
			start_loading();
			status = get_temperature();
			stop_loading();
			
#ifdef SHOW_MANUAL
			if (status == ESP8266_SUCCESS) {
				char weather_info[50];
				strcpy(weather_info, "WEATHER FOR TRONDHEIM NORWAY: ");
				strcat(weather_info,env_var.temperature);
				strcat(weather_info,"C, ");
				strcat(weather_info,toupper(env_var.weather_info));
				ht1632c_scroll_print(weather_info, false);
			} else {
				ht1632c_scroll_print("COULD NOT GET TEMPERATURE...", false);
			}
#endif
			menu_draw_temperature_frame();
			ht1632c_slide_in_from_top();
			
		} else if (env_var.menu_id == 2) {
			ht1632c_slide_out_to_top();
			get_week();
			menu_draw_week_frame();
			ht1632c_slide_in_from_top();
		}
	}
}

uint8_t menu_configuration(void) {
	
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
		ht1632c_slide_out_to_right();
	} else if (btn_status == BTN4) {
		rtc_disable_time_render();
		ht1632c_slide_out_to_left();
	} else if (btn_status == BTN3) {
		ht1632c_fade_blink();
	}
	
	//Switch between menus
	if (btn_status != NONE) {
		switch(env_var.menu_id) {
			case 0:
				ht1632c_draw_four_letter_word("LGHT");
				break;
			case 1:
				ht1632c_draw_four_letter_word("TIME");
				break;
			case 2:
				ht1632c_draw_four_letter_word("WIFI");
				break;
			case 3:
				ht1632c_draw_four_letter_word("ALRM");
				break;
			case 4:
				ht1632c_draw_four_letter_word("EXIT");
				break;
			default:
				break;
		}	
	}
	
	if (btn_status == BTN1) {
		ht1632c_slide_in_from_left();
	} else if (btn_status == BTN4) {
		ht1632c_slide_in_from_right();
	}
	
	//Menu actions
	if (btn_status == BTN3) {
		//Used for wifi
		esp8266_status_t status;
		switch(env_var.menu_id) {
			case 0:
				ht1632c_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				ht1632c_scroll_print("USE BUTTONS TO ADJUST BRIGHTNESS",false);
#endif
				menu_configure_brightnesss();
				ht1632c_set_brightness(MAX_BRIGHTNESS);
				break;
			case 1:
				ht1632c_slide_out_to_bottom();
#ifdef SHOW_MANUAL
				ht1632c_scroll_print("USE BUTTONS TO SET THE TIME",false);
				_delay_ms(500);
#endif
				menu_set_time();
				break;
			case 2:
				esp8266_on();
#ifdef SHOW_MANUAL
				ht1632c_scroll_print("STARTING TELNET SERVICE",false);
#endif
				status = esp8266_setup_webserver(true, false);
				if (status == ESP8266_SUCCESS) {
					//EDIT: Removed this, takes much RAM
					//esp8266_telnet_server();
				} else {
#ifdef SHOW_MANUAL
					ht1632c_scroll_print("ERROR: COULD NOT START TELNET SERVICE",false);
#endif
				}
				break;
			case 3:
#ifdef SHOW_MANUAL
				ht1632c_scroll_print("USE BUTTONS TO SET THE ALARM",false);
				_delay_ms(500);
#endif
				ht1632c_slide_out_to_bottom();
				menu_set_alarm();
			case 4:
				ht1632c_slide_out_to_bottom();
				return 0;
			default:
				break;
		}
		ht1632c_slide_out_to_bottom();
		ht1632c_draw_four_letter_word("<  >");
		ht1632c_slide_in_from_top();
	}	
	return 1;
}

void menu_configure_brightnesss(void) {
	
	bool quit = false;
	
	ht1632c_set_brightness(env_var.brightness);
	ht1632c_draw_filled_rect(7,0,2,env_var.brightness,1);
	ht1632c_refresh_screen();
	
	while(!quit) {
		btn_status = btn_check_press();

		switch(btn_status) {
			case BTN1:
				if (env_var.brightness < 15) {
					ht1632c_draw_filled_rect(7,0,2,env_var.brightness,0);
					ht1632c_set_brightness(++env_var.brightness);
					//printf("Brightness: %d \n",env_var.brightness);
					ht1632c_draw_filled_rect(7,0,2,env_var.brightness,1);
					ht1632c_refresh_screen();
				}
				_delay_ms(100);
				break;
			case BTN2:
				if (env_var.brightness > 1) {
					ht1632c_draw_filled_rect(7,0,2,env_var.brightness,0);
					ht1632c_set_brightness(--env_var.brightness);
					//printf("Brightness: %d \n",env_var.brightness);
					ht1632c_draw_filled_rect(7,0,2,env_var.brightness,1);
					ht1632c_refresh_screen();
				}
				_delay_ms(100);
				break;
			case BTN4:
				ht1632c_clear_screen();
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
	
	ht1632c_draw_three_letter_word("HRS");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	rtc_update_display(5,env_var.time.hours);
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				rtc_increment_hour();
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				rtc_decrement_hour();
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
	ht1632c_slide_out_to_left();
	ht1632c_draw_three_letter_word("MIN");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	rtc_update_display(5,env_var.time.minutes);
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set minutes
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				rtc_increment_minute();
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				rtc_decrement_minute();
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
	ht1632c_slide_out_to_left();
	ht1632c_draw_three_letter_word("SEC");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	rtc_update_display(5,env_var.time.seconds);
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set seconds
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				rtc_increment_second();
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				rtc_decrement_second();
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
	ht1632c_slide_out_to_left();
	ht1632c_draw_three_letter_word("DAY");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	ht1632c_draw_three_letter_word(get_day_name(env_var.time.weekday));
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set day
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				rtc_increment_day();
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				rtc_decrement_day();
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
	ht1632c_slide_out_to_left();
	ht1632c_draw_four_letter_word("MNTH");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	ht1632c_draw_three_letter_word(get_month_name(env_var.time.month));
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set month
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				rtc_increment_month();
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				rtc_decrement_month();
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
	ht1632c_slide_out_to_left();
	ht1632c_draw_four_letter_word("YEAR");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	char *year;
	itoa_simple(year,env_var.time.year);
	ht1632c_draw_four_letter_word(year);
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set year
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				rtc_increment_year();
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				rtc_decrement_year();
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
	ht1632c_scroll_print("TIME AND DATE IS NOW UPDATED",false);
#endif
	
	return 1;
}

uint8_t menu_set_alarm(void) {
	
	bool quit = false;
	bool next = false;
	
	ht1632c_draw_three_letter_word("HRS");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	ht1632c_increment_hour(env_var.alarm.hours);
	ht1632c_increment_minute(env_var.alarm.minutes);
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set hours
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				ht1632c_increment_hour(env_var.alarm.hours++);
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				ht1632c_decrement_hour(env_var.alarm.hours--);
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
	ht1632c_slide_out_to_left();
	ht1632c_draw_three_letter_word("MIN");
	ht1632c_slide_in_from_right();
	_delay_ms(1000);
	ht1632c_slide_out_to_left();
	rtc_update_display(5,env_var.time.minutes);
	ht1632c_slide_in_from_right();
	ht1632c_blink(true);
	
	//Set minutes
	while(!next) {
		btn_status = btn_check_press();
		switch(btn_status) {
			case BTN1:
				ht1632c_clear_screen();
				ht1632c_increment_minute(env_var.alarm.minutes++);
				_delay_ms(250);
				break;
			case BTN2:
				ht1632c_clear_screen();
				ht1632c_decrement_minute(env_var.alarm.minutes--);
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
	ht1632c_scroll_print("ALARM IS NOW SET",false);
#endif
	
	return 1;
}

ISR(TCC0_CCA_vect) {
	ht1632c_loading2();
}

ISR(TCC0_CCB_vect) {
	//ht1632c_loading3();
}