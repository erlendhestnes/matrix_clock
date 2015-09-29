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

#define NUMBER_OF_MENUS 4
#define NO_ACTION		0xff

typedef enum {
	MENU_ERROR,
	MENU_SUCCESS,
	MENU_CLOSE
} menu_status_t;

void menu_set_env_variables(void);
menu_status_t menu_configuration(void);
menu_status_t menu_state_machine(SI114X_IRQ_SAMPLE *samples);
void menu_configure_brightnesss(void);

uint8_t menu_set_time(void);
uint8_t menu_set_alarm(void);

#endif /* MENU_H_ */