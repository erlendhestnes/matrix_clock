/*
 * dac.h
 *
 * Created: 1/31/2016 3:07:31 PM
 *  Author: Administrator
 */ 


#ifndef DAC_H_
#define DAC_H_

#include "../../global.h"

#define DAC_PORT	PORTB
#define DAC0		PIN2_bm
#define DAC1		PIN3_bm

void dac_speaker_on(void);
void dac_speaker_off(void);
void dac_setup(bool dual_channel);
void dac_off(void);

static inline void dac_ch0_write(uint16_t data) {
	DACB.CH0DATA = data;
}

static inline void dac_ch1_write(uint16_t data) {
	DACB.CH1DATA = data;
}

#endif /* DAC_H_ */