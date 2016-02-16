/*
 * adc.h
 *
 * Created: 1/18/2015 4:16:28 PM
 *  Author: Administrator
 */ 

#ifndef _ADC_H_
#define _ADC_H_

#include "../../global.h"

#define CURRENT_MEASUREMENT_ENABLE PIN1_bm

void adc_setup(void);
void adc_disable(void);

void adc_enable_current_measurement(void);
void adc_disable_current_measurement(void);

uint16_t adc_read_voltage(void);
float adc_get_battery_voltage(void);
uint8_t adc_get_battery_percentage(void);

#endif