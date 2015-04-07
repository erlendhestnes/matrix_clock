/*
 * adc.c
 *
 * Created: 1/18/2015 4:16:21 PM
 *  Author: Administrator
 */ 
#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"

void adc_setup(void) {
	ADCA.CTRLA = ADC_FLUSH_bm; // cancel any pending conversions, disable ADC
	// set up exactly how Atmel did when they measured the calibration value
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc; // unsigned conversion, produces result in range 0-2048
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_TEMPREF_bm;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc | ADC_CH_GAIN_1X_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXINT_TEMP_gc;
	ADCA.CTRLA |= ADC_ENABLE_bm;
}

uint8_t read_signature_byte(uint16_t Address) {
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	__asm__ ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}

uint16_t adc_read(uint8_t ch, uint8_t mode) // Mode = 1 for single ended, 0 for internal
{
	if ((ADCA.CTRLA & ADC_ENABLE_bm) == 0)
	{
		ADCA.CTRLA = ADC_ENABLE_bm ; // Enable the ADC
		ADCA.CTRLB = (1<<4); // Signed Mode
		ADCA.REFCTRL = 0; // Internal 1v ref
		ADCA.EVCTRL = 0 ; // no events
		ADCA.PRESCALER = ADC_PRESCALER_DIV128_gc ;
		ADCA.CALL = read_signature_byte(0x20) ; //ADC Calibration Byte 0
		ADCA.CALH = read_signature_byte(0x21) ; //ADC Calibration Byte 1
		//ADCA.SAMPCTRL = This register does not exist
		_delay_us(400); // Wait at least 25 clocks
	}
	ADCA.CH0.CTRL = ADC_CH_GAIN_1X_gc | mode ; // Gain = 1, Single Ended
	ADCA.CH0.MUXCTRL = (ch<<3);
	ADCA.CH0.INTCTRL = 0 ; // No interrupt
	//ADCA.CH0.SCAN Another bogus register
	for(uint8_t Waste = 0; Waste<2; Waste++)
	{
		ADCA.CH0.CTRL |= ADC_CH_START_bm; // Start conversion
		while (ADCA.INTFLAGS==0) ; // Wait for complete
		ADCA.INTFLAGS = ADCA.INTFLAGS ;
	}
	return ADCA.CH0RES ;
}

int16_t adc_get_temp(void) {
	
	int16_t ref; 
	int16_t degrees_x10;
	float kelvin_per_adc_x10;
	
	ADCA.CTRLA = ADC_FLUSH_bm; // cancel any pending conversions, disable ADC
	// set up exactly how Atmel did when they measured the calibration value
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc; // unsigned conversion, produces result in range 0-2048
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_TEMPREF_bm;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc | ADC_CH_GAIN_1X_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXINT_TEMP_gc;
	ADCA.CTRLA |= ADC_ENABLE_bm;
	// adc_wait_8mhz();
	_delay_us(400); // Wait at least 25 clocks
	// get 358 K factory calibrated value
	//ref = read_signature_byte(PROD_SIGNATURES_START + TEMPSENSE0);
	//ref += read_signature_byte(PROD_SIGNATURES_START + TEMPSENSE1) << 8;
	
	kelvin_per_adc_x10 = ((273 + 85)*10) / (float)ref; // reference is ADC reading at 85C, scaled by 10 to get units of 0.1C
	degrees_x10 = ADCA.CH0RES;//ADC_sample(&ADCA.CH0);
	ADCA.CTRLA = 0; // turn ADC off
	ADCA.REFCTRL = 0; // turn temperature sensor off
	degrees_x10 *= kelvin_per_adc_x10;
	degrees_x10 -= 2730;
	
	return (degrees_x10);
}