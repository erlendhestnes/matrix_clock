/*
 * matrix_clock.c
 *
 * Created: 10/9/2014 5:49:41 PM
 *  Author: Administrator
 */ 

#define F_CPU 32000000UL

#define RAND_MAX 255

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/sleep.h>

#include "ht1632c.h"
#include "rtc.h"
#include "sercom.h"
#include "uart.h"
#include "fatfs/ff.h"
#include "fatfs/sound.h"
#include "fft/fft.h"
#include "power.h"
#include "esp8266.h"
#include "port.h"
#include "usb.h"

#include "si114x/User_defs.h"
#include "si114x/Si114x_functions.h"
#include "si114x/Si114x_handler.h"

static volatile uint16_t counter = 0;

FATFS FatFs;		// FatFs work area needed for each volume
FIL Fil;			// File object needed for each open file
BYTE Buff[2048];	// Working buffer

void uart_put_char(char c);
static FILE mystdout = FDEV_SETUP_STREAM(uart_put_char,uart_get_char,_FDEV_SETUP_WRITE);

void pmic_setup(void) {
	
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
}

void play_sound(char *name) {
	
	BYTE res;
	res = f_open(&Fil, "john3.wav", FA_READ);
	if (!res) {
		load_wav(&Fil, "**** WAV PLAYER ****", Buff, sizeof Buff);
		f_close(&Fil);
	}	
}

int main(void) {

	UINT bw;
	char *ptr;
	
	clock_setup_32_mhz();
	ht1632c_begin(HT1632_COMMON_16NMOS);
	ht1632c_setBrightness(15);
	ht1632c_clearScreen();
	//ht1632c_fillScreen();
	uart_setup();
	pmic_setup();
	i2c_setup();
	btn_setup();
	rtc_setup();
	
	stdout = stdin = &mystdout;

	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
	
	if (f_open(&Fil, "newfilea.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
		
		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
		
		f_close(&Fil);								/* Close the file */
	}
	
	SI114X_IRQ_SAMPLE sensor_data;
	
	//_delay_ms(5000);
	
	si114x_reset(SI114X_ADDR);
	//si114x_init(SI114X_ADDR);
	
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC1.PERL = 0x80;
	TCC1.PERH = 0x0C;
	TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
	
	init_time();
	
	sei();
	
	while (1) {
		
		update_time();
		
		//sensor_data.timestamp = counter;
		//si114x_get_data(&sensor_data);
		//si114x_process_samples(SI114X_ADDR,&sensor_data);
	}
	
}

//Used for SI114x Timestamp
ISR(TCC1_OVF_vect) {
	counter++;	
}