/*
 * port.c
 *
 * Created: 3/25/2015 3:00:46 PM
 *  Author: Administrator
 */ 

#include "port.h"
#include "../ht1632c/ht1632c.h"

#define BUTTON0 PIN5_bm
#define BUTTON1 PIN6_bm
#define BUTTON2 PIN7_bm
#define BUTTON3 PIN0_bm

void btn_setup(void) {
	
	PORTA.DIRCLR = BUTTON0 | BUTTON1 | BUTTON2;
	PORTB.DIRCLR = BUTTON3;
	
	PORTA.PIN5CTRL = PORT_ISC_FALLING_gc;
	PORTA.PIN6CTRL = PORT_ISC_FALLING_gc;
	PORTA.PIN7CTRL = PORT_ISC_FALLING_gc;
	
	PORTB.PIN0CTRL = PORT_ISC_FALLING_gc;
	
	PORTA.INT0MASK = BUTTON0 | BUTTON1 | BUTTON2;
	PORTB.INT0MASK = BUTTON3;
	
	PORTA.INTCTRL = PORT_INT0LVL_HI_gc;
	PORTB.INTCTRL = PORT_INT0LVL_HI_gc;

}

void btn_top_setup(void) {
	
	PORTC.DIRCLR = PIN2_bm;
	PORTC.PIN2CTRL = PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
	PORTC.INT0MASK = PIN2_bm;
	PORTC.INTCTRL = PORT_INT0LVL_HI_gc;
	
}

button_t btn_check_press(void) {
	
	if (!(PORTA.IN & BUTTON0)) {
		uwrite_hex(DACB.CH0OFFSETCAL);
		DACB.CH0OFFSETCAL += 1;
		return BTN1;
	} else if (!(PORTA.IN & BUTTON1)) {
		uwrite_hex(DACB.CH0OFFSETCAL);
		DACB.CH0OFFSETCAL -= 1;
		return BTN2;
	} else if (!(PORTA.IN & BUTTON2)) {
		uwrite_hex(DACB.CH0GAINCAL);
		DACB.CH0GAINCAL += 1;
		return BTN3;
	} else if (!(PORTB.IN & BUTTON3)) {
		uwrite_hex(DACB.CH0GAINCAL);
		DACB.CH0GAINCAL -= 1;
		return BTN4;
	} else {
		return NONE;
	}
}

ISR(PORTA_INT0_vect) {
	btn_status = btn_check_press();
	//printf("%d",btn_status);
}

ISR(PORTB_INT0_vect) {
	btn_status = btn_check_press();
	//printf("%d",btn_status);
}