/*
 * adc.c
 *
 * Created: 1/18/2015 4:16:21 PM
 *  Author: Administrator
 */ 
#include <avr/io.h>
#include "adc.h"

void adc_init(void) {
	ADCA.CTRLA = ADC_FLUSH_bm; // cancel any pending conversions, disable ADC
	// set up exactly how Atmel did when they measured the calibration value
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc; // unsigned conversion, produces result in range 0-2048
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_TEMPREF_bm;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc | ADC_CH_GAIN_1X_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXINT_TEMP_gc;
	ADCA.CTRLA |= ADC_ENABLE_bm;
}
