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
	NO_BTN,
	BTN1,
	BTN2,
	BTN3,
	BTN4,
	BTN5
} button_t;

typedef enum {
	ALS_INT_1 = 1,
	ALS_INT_2 = 2,
	PS1_INT = 4
} si114x_int_t;

#define INTERRUPT_MODE true
#define POLL_MODE	   false

volatile button_t btn_status;
volatile si114x_int_t si114x_status;

void btn_setup(bool enable_interrupt);
void btn_top_setup(void);
void btn_si114x_enable_interrupt(void);
void btn_disable_si114x_interrupt(void);
void btn_turn_on_leds(bool leds_on);

button_t btn_check_press(void);

#endif