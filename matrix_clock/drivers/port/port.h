/*
 * port.h
 *
 * Created: 3/25/2015 3:00:54 PM
 *  Author: Administrator
 */ 

#ifndef PORT_H_
#define PORT_H_

#include "../../global.h"

typedef enum {
	NONE,
	BTN1,
	BTN2,
	BTN3,
	BTN4,
	BTN5
} button_t;

volatile button_t btn_status;

void btn_setup(void);
void btn_top_setup(void);
button_t btn_check_press(void);

#endif