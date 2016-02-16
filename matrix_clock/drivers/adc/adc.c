#include "adc.h"

int16_t offset = 0;

void adc_setup(void) 
{	
	unsigned char samples = 16;
	
	//Disable power reduction for ADCA 
	PR.PRPA &= ~0x02;
	
	ADCA.CALL = read_signature_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
	ADCA.CALH = read_signature_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );

	ADCA.CH0.CTRL	 = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
	ADCA.CTRLB		 = ADC_RESOLUTION_12BIT_gc;
	ADCA.PRESCALER	 = ADC_PRESCALER_DIV256_gc;
	ADCA.REFCTRL	 = ADC_REFSEL_INTVCC_gc;
	ADCA.EVCTRL		 = ADC_EVACT_NONE_gc;
	ADCA.INTFLAGS	 = ADC_CH0IF_bm;
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_OFF_gc;
	ADCA.CTRLA		 = ADC_ENABLE_bm;
	_delay_ms(4);
	
	while (samples > 0) {
		ADCA.CH0.CTRL |= (1 << ADC_CH_START_bp);
		while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
		ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
		offset += ADCA.CH0.RES;
		samples--;
	}
	
	ADCA.CTRLA &= ~(ADC_ENABLE_bm);
	offset >>= 4;
	ADCA.CMP = 0x0000;
	ADCA.CH0.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
	ADCA.EVCTRL	= ADC_SWEEP_0_gc | ADC_EVACT_NONE_gc;
	ADCA.CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_OFF_gc;
	ADCA.CH1.INTCTRL = ADC_CH_INTLVL_OFF_gc;
	ADCA.CH2.INTCTRL = ADC_CH_INTLVL_OFF_gc;
	ADCA.CH3.INTCTRL = ADC_CH_INTLVL_OFF_gc;
	
	ADCA.CTRLA |= ADC_ENABLE_bm;
	_delay_ms(10);
}

void adc_disable(void) 
{	
	ADCA.CTRLA &= ~(ADC_ENABLE_bm);
	
	//Enable power reduction for ADCA
	PR.PRPA |= 0x02;
}

uint16_t adc_read_voltage(void) 
{	
	adc_enable_current_measurement();
	uint32_t val = 0;
	for (uint8_t i = 0; i < 25; i++) {
		ADCA.CH0.CTRL |= (1 << ADC_CH_START_bp);
		while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
		ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
		val += ADCA.CH0.RES;
	}
	adc_disable_current_measurement();

	return val/25;
}

float adc_get_battery_voltage(void) 
{
	adc_setup();
	
	float offset_error = 0.33;
	float measured_voltage;
	float battery_voltage;
	
	measured_voltage = (float)adc_read_voltage();
	battery_voltage = ((measured_voltage * 2.05f)/(4095.0f)) * 32.0f/10.0f;
	
	adc_disable();
	
	return (battery_voltage - offset_error);
}

uint8_t adc_get_battery_percentage(void) 
{	
	uint16_t voltage;
	float battery_voltage;
	
	battery_voltage = (adc_get_battery_voltage() * 100.0f);
	voltage = (uint16_t)battery_voltage;
	
	if (voltage > 600) {
		return 99;
	} else if (voltage > 575) {
		return 95;
	} else if (voltage > 550) {
		return 90;
	} else if (voltage > 525) {
		return 80;
	} else if (voltage > 500) {
		return 70;
	} else if (voltage > 475) {
		return 60;
	} else if (voltage > 450) {
		return 50;
	} else if (voltage > 425) {
		return 40;
	} else if (voltage > 415) {
		return 30;
	} else if (voltage > 400) {
		return 20;
	} else if (voltage > 375) {
		return 10;
	} else if (voltage > 350) {
		return 5;
	}
	
	return 0;
}

void adc_enable_current_measurement(void) 
{	
	PORTA.DIRSET = CURRENT_MEASUREMENT_ENABLE;
	PORTA.OUTCLR = CURRENT_MEASUREMENT_ENABLE;
	
	//Test
	//PORTA.DIRCLR = PIN1_bm;
	//PORTA.PIN1CTRL = PORT_OPC_PULLUP_gc
}

void adc_disable_current_measurement(void) 
{	
	PORTA.DIRSET = CURRENT_MEASUREMENT_ENABLE;
	PORTA.OUTSET = CURRENT_MEASUREMENT_ENABLE;
	
	//Test
	//PORTA.DIRCLR = PIN1_bm;
	//PORTA.PIN1CTRL = PORT_OPC_PULLUP_gc;
}