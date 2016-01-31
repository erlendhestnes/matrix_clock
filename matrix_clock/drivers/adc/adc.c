#include "adc.h"

void adc_init(void) 
{	
	PR.PRPA &= ~0x02; // Clear ADC bit in Power Reduction Port B Register
	
	ADCA.CALL = 0x36;
	ADCA.CALH = 0x03;
	//ADCA.CALL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
	//ADCA.CALH = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );
	//ADCA.CALL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
	//ADCA.CALH = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );	

	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
	ADCA.PRESCALER = ADC_PRESCALER_DIV256_gc;
	ADCA.REFCTRL = ADC_REFSEL_INTVCC_gc;
	ADCA.EVCTRL = ADC_EVACT_NONE_gc;
	ADCA.INTFLAGS = ADC_CH0IF_bm;
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_OFF_gc;
	ADCA.CTRLA = ADC_ENABLE_bm;
}

void adc_disable(void) 
{	
	PR.PRPA |= 0x02;
	ADCA.CTRLA &= ~(ADC_ENABLE_bm);
}

uint16_t adc_read_voltage(void) 
{	
	adc_enable_current_measurement();
	uint16_t val = 0;
	for (uint8_t i = 0; i < 10; i++) {
		ADCA.CH0.CTRL |= (1 << ADC_CH_START_bp);
		while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
		ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
		val += ADCA.CH0.RES;
	}
	adc_disable_current_measurement();

	return val/10;
}

uint8_t adc_get_battery_percentage(void) 
{	
	adc_init();

	float voltage;
	voltage = (float)adc_read_voltage();
	voltage /= 6.6f;
	
	adc_disable();
	
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
	}
	
	return 0;
}

void adc_enable_current_measurement(void) 
{	
	PORTA.DIRSET |= PIN1_bm;
	PORTA.OUTCLR |= PIN1_bm;
}

void adc_disable_current_measurement(void) 
{	
	PORTA.DIRSET |= PIN1_bm;
	PORTA.OUTSET |= PIN1_bm;
}

uint8_t read_calibration_byte(uint8_t index) 
{	
	uint8_t result;
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);
	
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	
	return(result);
}