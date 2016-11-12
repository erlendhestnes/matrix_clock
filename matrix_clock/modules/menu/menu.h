/*
 * menu.h
 *
 * Created: 5/1/2015 10:35:50 PM
 *  Author: Administrator
 */ 


#ifndef MENU_H_
#define MENU_H_

#include "../../global.h"
#include "../../drivers/sensors/si114x/Si114x_types.h"

#define USER_SSID        "crosstalk"
#define USER_PASS        "9A1660041E"

#define NUMBER_OF_MENUS 6
#define NUMBER_OF_CONFIG_MENUS 7
#define NO_ACTION		0xff

typedef enum {
	MENU_TIME,
	MENU_CONFIG,
	MENU_BATTERY,
	MENU_TEMP,
	MENU_WEEK,
	MENU_DATE,
	MENU_WEEKDAY
} menu_items_t;

typedef enum {
	CONFIG_EXIT,
	CONFIG_INFO,
	CONFIG_MUSIC,
	CONFIG_WIFI,
	CONFIG_CALIBRATE,
	CONFIG_BRIGHTNESS,
	CONFIG_ALARM,
	CONFIG_TIME
} config_times_t;

typedef enum {
	MENU_ERROR,
	MENU_SUCCESS,
	MENU_CLOSE
} menu_status_t;

menu_status_t get_internet_variables(bool get_time, bool get_temperature);
menu_status_t get_access_points(void);

menu_status_t menu_configuration(SI114X_IRQ_SAMPLE *samples);
menu_status_t menu_state_machine(SI114X_IRQ_SAMPLE *samples);

void	menu_set_env_variables(void);
void	menu_configure_brightnesss(void);
uint8_t menu_set_time(void);
uint8_t menu_set_alarm(void);
uint8_t menu_yes_no(void);
void menu_type_wifi_password(void);

void start_wifi_indication(void);
void stop_wifi_indication(void);

void menu_alarm(void);

void play_sound(char *str);

menu_status_t menu_esp8266_setup(void);

#endif /* MENU_H_ */