/*
 * clock.h
 *
 * Created: 12/20/2014 1:41:56 PM
 *  Author: Administrator
 */ 

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "../../global.h"

void clock_setup_1_mhz(void);
void clock_setup_8_mhz(void);
void clock_setup_16_mhz(void);
void clock_setup_32_mhz(void);

void clock_setup_32_mhz_pll(void);
void clock_setup_48_mhz_pll(void);

#endif