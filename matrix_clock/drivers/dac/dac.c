/*
 * dac.c
 *
 * Created: 1/31/2016 3:07:22 PM
 *  Author: Administrator
 */ 

#include "dac.h"

void dac_speaker_on(void) 
{
	PORTD.DIRSET = SPEAKER_EN;
	PORTD.OUTSET = SPEAKER_EN;
}

void dac_speaker_off(void) 
{
	PORTD.DIRSET = SPEAKER_EN;
	PORTD.OUTCLR = SPEAKER_EN;
}

void dac_setup(bool dual_channel) 
{
	//Disable power reduction for DACB
	PR.PRPB &= ~0x04;
			
	if (dual_channel)
	{
		PORTB.DIRSET = DAC0 | DAC1;
		
		DACB.CTRLA |= DAC_CH0EN_bm | DAC_CH1EN_bm;
		DACB.CTRLC |= DAC_REFSEL_AVCC_gc;
		DACB.CTRLB |= DAC_CHSEL_DUAL_gc;
	} else {
		PORTB.DIRSET = DAC0;
		
		DACB.CTRLA |= DAC_CH0EN_bm;
		DACB.CTRLC |= DAC_REFSEL_AVCC_gc;
		DACB.CTRLB |= DAC_CHSEL_SINGLE_gc;
	}
	
	DACB.CH0OFFSETCAL = read_signature_byte( offsetof(NVM_PROD_SIGNATURES_t, DACB0OFFCAL) );
	DACB.CH0GAINCAL = read_signature_byte( offsetof(NVM_PROD_SIGNATURES_t, DACB0GAINCAL) );
	
	DACB.CH1OFFSETCAL = read_signature_byte( offsetof(NVM_PROD_SIGNATURES_t, DACB1OFFCAL) );
	DACB.CH1GAINCAL = read_signature_byte( offsetof(NVM_PROD_SIGNATURES_t, DACB1GAINCAL) );
	
	DACB.CTRLA |= DAC_ENABLE_bm;
	
	dac_speaker_on();
}

void dac_disable(void) 
{
	PORTB.DIRCLR = DAC0 | DAC1;
	
	PORTB.PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTB.PIN3CTRL = PORT_OPC_PULLUP_gc;
	
	DACB.CTRLA &= ~(DAC_ENABLE_bm);
	dac_speaker_off();
	
	//Enable power reduction for DACB
	PR.PRPB |= 0x04;
}