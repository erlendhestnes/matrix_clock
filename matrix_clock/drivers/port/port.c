/*
 * port.c
 *
 * Created: 3/25/2015 3:00:46 PM
 *  Author: Administrator
 */ 

#include "port.h"
#include "../ht1632c/ht1632c.h"
#include "../sensors/si114x/Si114x_functions.h"

#define BUTTON0 PIN5_bm
#define BUTTON1 PIN6_bm
#define BUTTON2 PIN7_bm
#define BUTTON3 PIN0_bm

#define SI114X_INT PIN2_bm

void btn_setup(bool enable_interrupt) 
{	
	PORTA.DIRCLR |= BUTTON0 | BUTTON1 | BUTTON2;
	PORTB.DIRCLR |= BUTTON3;
	
	if (enable_interrupt)
	{
		PORTA.PIN5CTRL |= PORT_ISC_FALLING_gc | PORT_OPC_PULLUP_gc;
		PORTA.PIN6CTRL |= PORT_ISC_FALLING_gc | PORT_OPC_PULLUP_gc;
		PORTA.PIN7CTRL |= PORT_ISC_FALLING_gc | PORT_OPC_PULLUP_gc;
		
		PORTB.PIN0CTRL |= PORT_ISC_FALLING_gc | PORT_OPC_PULLUP_gc;
		
		PORTA.INT0MASK |= BUTTON0 | BUTTON1 | BUTTON2;
		PORTB.INT0MASK |= BUTTON3;
		
		PORTA.INTCTRL |= PORT_INT0LVL_HI_gc;
		PORTB.INTCTRL |= PORT_INT0LVL_HI_gc;
	} else {
		PORTA.PIN5CTRL &= ~(PORT_ISC_FALLING_gc);
		PORTA.PIN6CTRL &= ~(PORT_ISC_FALLING_gc);
		PORTA.PIN7CTRL &= ~(PORT_ISC_FALLING_gc);
		
		PORTB.PIN0CTRL &= ~(PORT_ISC_FALLING_gc);
		
		PORTA.INT0MASK &= ~(BUTTON0 | BUTTON1 | BUTTON2);
		PORTB.INT0MASK &= ~(BUTTON3);
		
		PORTA.INTCTRL &= ~(PORT_INT0LVL_HI_gc);
		PORTB.INTCTRL &= ~(PORT_INT0LVL_HI_gc);
	}
}

void btn_si114x_enable_interrupt(void) 
{	
	PORTA.DIRCLR |= SI114X_INT;
	PORTA.PIN2CTRL |= PORT_ISC_FALLING_gc;
	PORTA.INT1MASK |= SI114X_INT;
	PORTA.INTCTRL |= PORT_INT1LVL_HI_gc;
}

void btn_si114x_disable_interrupt(void) 
{	
	PORTA.DIRCLR &= ~(SI114X_INT);
	PORTA.PIN2CTRL &= ~(PORT_ISC_FALLING_gc);
	PORTA.INT1MASK &= ~(SI114X_INT);
	PORTA.INTCTRL &= ~(PORT_INT1LVL_HI_gc);
}

void btn_top_setup(void) 
{	
	PORTC.DIRCLR |= PIN2_bm;
	PORTC.PIN2CTRL |= PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
	PORTC.INT0MASK |= PIN2_bm;
	PORTC.INTCTRL |= PORT_INT0LVL_HI_gc;
}

button_t btn_check_press(void) 
{	
	button_t btn_mask = NO_BTN;
	
	if (!(PORTA.IN & BUTTON0)) {
		btn_mask |= BTN1;
	}
	if (!(PORTA.IN & BUTTON1)) {
		btn_mask |= BTN2;
	}
	if (!(PORTA.IN & BUTTON2)) {
		btn_mask |= BTN3;
	} 
	if (!(PORTB.IN & BUTTON3)) {
		btn_mask |= BTN4;
	}
	return btn_mask;
}

ISR(PORTA_INT0_vect) 
{
	btn_status = btn_check_press();
	printf("%d",btn_status);
}

ISR(PORTB_INT0_vect) 
{
	btn_status = btn_check_press();
	printf("%d",btn_status);
}

ISR(PORTA_INT1_vect) 
{	
	uint8_t data[2];
	twi_read_packet(&TWIC,SI114X_ADDR,50,REG_IRQ_STATUS,data,1);
	si114x_status = data[0];
	//data[0] = 0x00;
	//twi_write_packet(&TWIC,SI114X_ADDR,50,REG_IRQ_STATUS,data,1);

	puts("Interrupt from Si114x! \n");
}