/*
 * timer.c
 *
 * Created: 1/31/2016 2:34:09 PM
 *  Author: Administrator
 */ 

#include "timer.h"
#include "../dac/dac.h"

static uint16_t test_val = 0;

void timer_setup(void) 
{
	TCC0.CNT = 0;
	TCC0.PER = 125;
	TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
	
	TCC0.CCC = 125;
	TCC0.INTCTRLB |= TC_CCCINTLVL_LO_gc;
	TCC0.CTRLB |= TC0_CCCEN_bm;
}

void timer_audio_test(void) 
{
	dac_setup(false);
	timer_setup();
}

ISR(TCC0_CCC_vect) 
{
	if (test_val == 1)
	{
		DACB.CH0DATA = 0xfff;
		test_val = 0;
	} else {
		DACB.CH0DATA = 0x000;
		test_val = 1;
	}
	
}
