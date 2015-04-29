/*
 * port.c
 *
 * Created: 3/25/2015 3:00:46 PM
 *  Author: Administrator
 */ 

#include "port.h"
#include "ht1632c.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

void btn_setup(void) {
	
	PORTA.DIRCLR = PIN5_bm | PIN6_bm | PIN7_bm;
	PORTB.DIRCLR = PIN0_bm;
	
	//PORTA.PIN2CTRL = PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
	
	/*
	
	PORTA.INT0MASK = PIN5_bm | PIN6_bm | PIN7_bm;
	PORTB.INT0MASK = PIN0_bm;
	
	PORTA.INTCTRL = PORT_INT0LVL_LO_gc;
	PORTB.INTCTRL = PORT_INT0LVL_LO_gc;
	
	*/
}

void btn_top_setup(void) {
	
	PORTC.DIRCLR = PIN2_bm;
	PORTC.PIN2CTRL = PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
	PORTC.INT0MASK = PIN2_bm;
	PORTC.INTCTRL = PORT_INT0LVL_LO_gc;
}

button_t btn_check_press(void) {
	
	if (!(PORTA.IN & PIN5_bm)) {
		return BTN1;
	} else if (!(PORTA.IN & PIN6_bm)) {
		return BTN2;
	} else if (!(PORTA.IN & PIN7_bm)) {
		return BTN3;
	} else if (!(PORTB.IN & PIN0_bm)) {
		return BTN4;
	} else {
		return NONE;
	}
}

ISR(PORTA_INT0_vect) {
	ht1632c_clear_screen();
}