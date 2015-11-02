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

#define NUMBER_OF_MENUS 6
#define NUMBER_OF_CONFIG_MENUS 6
#define NO_ACTION		0xff

typedef enum {
	MENU_TIME,
	MENU_DATE,
	MENU_WEEKDAY,
	MENU_WEEK,
	MENU_TEMP,
	MENU_BATTERY,
	MENU_CONFIG
} menu_items_t;

typedef enum {
	CONFIG_TIME,
	CONFIG_ALARM,
	CONFIG_BRIGHTNESS,
	CONFIG_WIFI,
	CONFIG_CALIBRATE,
	CONFIG_INFO,
	CONFIG_EXIT
} config_times_t;

typedef enum {
	MENU_ERROR,
	MENU_SUCCESS,
	MENU_CLOSE
} menu_status_t;

void menu_set_env_variables(void);
menu_status_t menu_configuration(SI114X_IRQ_SAMPLE *samples);
menu_status_t menu_state_machine(SI114X_IRQ_SAMPLE *samples);

void menu_configure_brightnesss(void);
uint8_t menu_set_time(void);
uint8_t menu_set_alarm(void);

#endif /* MENU_H_ */