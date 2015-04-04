/*
 * fft.h
 *
 * Created: 12/20/2014 1:40:12 PM
 *  Author: Administrator
 */ 

// Include configuration and headers
#include "wff_fht/fhtConfig.h"
#include "sampling.h"

// Define an array for storing the input data to the FHT
#ifndef GENERATE_TABLES
int16_t fx[FHT_LEN];
#endif

void drawGraph64(int16_t *fx, int16_t len);
void runTests(uint16_t frequency, int16_t amplitude);