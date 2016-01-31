/*
 * dac.c
 *
 * Created: 1/31/2016 3:07:22 PM
 *  Author: Administrator
 */ 

#include "dac.h"

void dac_speaker_on(void) 
{
	PORTD.DIRSET |= PIN1_bm;
	PORTD.OUTSET |= PIN1_bm;
}

void dac_speaker_off(void) 
{
	PORTD.DIRSET |= PIN1_bm;
	PORTD.OUTCLR |= PIN1_bm;
}

void dac_setup(bool dual_channel) 
{		
	if (dual_channel)
	{
		DAC_PORT.DIRSET |= DAC0 | DAC1;
		
		DACB.CTRLA |= DAC_CH0EN_bm | DAC_CH1EN_bm;
		DACB.CTRLC |= DAC_REFSEL_AVCC_gc;
		DACB.CTRLB |= DAC_CHSEL_DUAL_gc;
	} else {
		DAC_PORT.DIRSET |= DAC0;
		
		DACB.CTRLA |= DAC_CH0EN_bm;
		DACB.CTRLC |= DAC_REFSEL_AVCC_gc;
		DACB.CTRLB |= DAC_CHSEL_SINGLE_gc;
	}
	
	//From calibration rows
	DACB.CH0OFFSETCAL = 0x07;
	DACB.CH0GAINCAL = 0x1B;
	DACB.CH1GAINCAL = 0x0C;
	DACB.CH1OFFSETCAL = 0x13;

	/*
	DACB.CH0OFFSETCAL = 0xE8;
	DACB.CH0GAINCAL = 0xB6;
	DACB.CH1GAINCAL = 0x0C;
	DACB.CH1OFFSETCAL = 0x13;
	*/
	
	DACB.CTRLA |= DAC_ENABLE_bm;
	
	dac_speaker_on();
}

void dac_off(void) 
{
	DAC_PORT.DIRCLR |= DAC0 | DAC1;
	
	DACB.CTRLA &= ~(DAC_ENABLE_bm);
	dac_speaker_off();
}

static inline uint8_t read_signature_byte(uint16_t Address) {
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	__asm__ ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}