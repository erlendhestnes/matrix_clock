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
#include <string.h>
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
#include "adc.h"
#include "json/jsmn.h"

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

void sd_card(void) {
	UINT bw;
	
	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
	
	if (f_open(&Fil, "newfilea.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
		
		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
		
		f_close(&Fil);								/* Close the file */
	}
}

void remove_substring(char *src, char *sub)
{
	char *p;
	if ((p=strstr(src,sub)) != NULL)
	{
		memmove(p,p+strlen(sub), strlen(p+strlen(sub))+1);

		// alternative
		// strcpy(p,p+strlen(sub));
	}
}

int main(void) {

	time_t user_time;
	SI114X_IRQ_SAMPLE sensor_data;
	jsmn_parser parser;
	
	jsmntok_t tokens[100];
	char *js;
	char *js2;
	char *js3;
	jsmnerr_t r;
	
	clock_setup_32_mhz();
	ht1632c_begin(HT1632_COMMON_16NMOS);
	ht1632c_setBrightness(0);
	ht1632c_clearScreen();
	//ht1632c_fillScreen();
	//adc_setup();
	uart_setup();
	pmic_setup();
	i2c_setup();
	btn_setup();
	rtc_setup();
	jsmn_init(&parser);
	
	stdout = stdin = &mystdout;
	
	sd_card();
	
	//_delay_ms(5000);
	
	si114x_reset(SI114X_ADDR);
	//si114x_init(SI114X_ADDR);
	
	//TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	//TCC1.PERL = 0x80;
	//TCC1.PERH = 0x0C;
	//TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
	
	//init_time();
	
	sei();
	
	
	//js = "sfdsf";
	//r = jsmn_parse(&parser, js, strlen(js), tokens, 256);

	esp8266_on();
	esp8266_setup();
	_delay_ms(5000);
	esp8266_off();
	js = strchr(rx_buffer,'{');
    js2 = strchr(js,'}');
	*js2++;

	remove_substring(js,js2);
	
	puts("DONE");
	puts(js);
	
	r = jsmn_parse(&parser, js, strlen(js), tokens, 100);
	
	puts("done");
	
	user_time.seconds = 0;
	user_time.minutes = 0;
	user_time.hours = 0;
	
	
	while (1) {
		
		/*
		update_time();
		
		switch(btn_check_press()) {
			case BTN1:
				user_time.minutes++;
				set_time(&user_time);
				_delay_ms(250);
				break;
			case BTN2:
				user_time.minutes--;
				set_time(&user_time);
				_delay_ms(250);
				break;
			case BTN3:
				user_time.hours++;
				set_time(&user_time);
				_delay_ms(250);
				break;
			case BTN4:
				user_time.hours--;
				set_time(&user_time);
				_delay_ms(250);
				break;
		}
		*/
		/*
		sensor_data.timestamp = counter;
		si114x_get_data(&sensor_data);
		si114x_process_samples(SI114X_ADDR,&sensor_data);
		*/
	}
	
}

//Used for SI114x Timestamp
ISR(TCC1_OVF_vect) {
	counter++;	
}