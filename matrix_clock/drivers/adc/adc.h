/*
 * adc.h
 *
 * Created: 1/18/2015 4:16:28 PM
 *  Author: Administrator
 */ 

#ifndef _ADC_H_
#define _ADC_H_

#include "../../global.h"

void adc_setup(void);
int16_t adc_get_temp(void);
uint16_t adc_read(uint8_t ch, uint8_t mode);

#endif