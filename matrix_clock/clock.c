/*
 * clock.c
 *
 * Created: 12/20/2014 1:41:49 PM
 *  Author: Administrator
 */ 

#include "clock.h"
#include <avr/io.h>

void clock_setup_1_mhz(void) {
	
	CCP = CCP_IOREG_gc;
	CLK.PSCTRL = CLK_PSADIV1_bm;
}

void clock_setup_32_mhz(void) {
	
	OSC.CTRL |= OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc; //Trigger protection mechanism
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; //Enable internal  32Mhz crystal
}

void clock_setup_16_mhz(void) {
	
	OSC.CTRL |= OSC_RC32MEN_bm; //Setup 32Mhz crystal
	
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	
	CCP = CCP_IOREG_gc; //Trigger protection mechanism
	CLK.PSCTRL = CLK_PSADIV0_bm;
	CCP = CCP_IOREG_gc; //Trigger protection mechanism
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; //Enable internal  32Mhz crystal
}

void clock_setup_8_mhz(void) {
	
	OSC.CTRL |= OSC_RC32MEN_bm; //Setup 32Mhz crystal
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc; //Trigger protection mechanism
	CLK.PSCTRL = CLK_PSADIV0_bm | CLK_PSADIV1_bm;
	CCP = CCP_IOREG_gc; //Trigger protection mechanism
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; //Enable internal  32Mhz crystal
}