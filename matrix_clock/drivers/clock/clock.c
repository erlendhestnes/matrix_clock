/*
 * clock.c
 *
 * Created: 12/20/2014 1:41:49 PM
 *  Author: Administrator
 */ 

#include "clock.h"

void clock_setup_1_mhz(void) 
{	
	CCP = CCP_IOREG_gc;
	CLK.PSCTRL = CLK_PSADIV1_bm;
}

void clock_setup_8_mhz(void) 
{	
	OSC.CTRL |= OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK.PSCTRL = CLK_PSADIV0_bm | CLK_PSADIV1_bm;
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}

void clock_setup_16_mhz(void) 
{	
	OSC.CTRL |= OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK.PSCTRL = CLK_PSADIV0_bm;
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}

void clock_setup_32_mhz(void) 
{	
	OSC.CTRL |= OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}

void clock_setup_32_mhz_pll(void) 
{	
	OSC.PLLCTRL = 0 | OSC_PLLFAC4_bm;
	OSC.CTRL |= OSC_PLLEN_bm;
	while ( !(OSC.STATUS & OSC_PLLEN_bm) ) ;
	CCP = CCP_IOREG_gc;
	CLK.CTRL = (CLK.CTRL & ~CLK_SCLKSEL_gm ) | CLK_SCLKSEL_PLL_gc;
}

//Overclocking, may work for some chips...
void clock_setup_48_mhz_pll(void) 
{	
	OSC.PLLCTRL = 0 | OSC_PLLFAC4_bm | OSC_PLLFAC3_bm;
	OSC.CTRL |= OSC_PLLEN_bm;
	while ( !(OSC.STATUS & OSC_PLLEN_bm) ) ;
	CCP = CCP_IOREG_gc;
	CLK.CTRL = (CLK.CTRL & ~CLK_SCLKSEL_gm ) | CLK_SCLKSEL_PLL_gc;
}