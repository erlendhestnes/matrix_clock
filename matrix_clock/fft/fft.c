/*
 * fft.c
 *
 * Created: 12/20/2014 1:40:04 PM
 *  Author: Administrator
 */ 

#include "fft.h"

void drawGraph64(int16_t *fx, int16_t len)
{
	int16_t i;

	// Draw a scaled graph of the output (y resolution = 0-63)
	for (i = 0; i < len; i++)
	{
		int16_t y;
		printf("%3d |", i);
		for (y = 0; y < fx[i]; y++) printf("*");
		printf("\r\n");
	}
}

void runTests(uint16_t frequency, int16_t amplitude)
{
	#ifndef GENERATE_TABLES

	generateSample(fx, frequency, amplitude);
	fhtDitInt(fx);
	complexToReal(fx, 7);
	drawGraph64(fx, FHT_LEN/2);

	#endif
}