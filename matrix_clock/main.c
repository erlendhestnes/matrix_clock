/*
 * matrix_clock.c
 *
 * Created: 10/9/2014 5:49:41 PM
 *  Author: Administrator
 */ 

#define F_CPU 32000000UL

#define RAND_MAX 255

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>

#include "ht1632c.h"
#include "rtc.h"
#include "sercom.h"
#include "uart.h"
#include "fatfs/ff.h"
#include "fatfs/sound.h"
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
BYTE Buff[2];	// Working buffer 2048

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

void print_token(jsmntok_t *tokens, char *js, uint8_t i) {
	int len;
	jsmntok_t key;

	key = tokens[i];
	len = key.end - key.start;
	char keyString[ len+1 ];
	memcpy( keyString, &js[ key.start ], len );
	keyString[ len ] = '\0';
	//printf( "Key[%d]: %s\n", i, keyString );
	puts(keyString);
}

void tcc_setup(void) {
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC1.PERL = 0x80;
	TCC1.PERH = 0x0C;
	TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
}

int main(void) {

	time_t user_time;
	
	SI114X_IRQ_SAMPLE sensor_data;
	
	jsmn_parser p;
	jsmntok_t tokens[100];
	jsmnerr_t r;
	char rx_buf[200];
	
	esp8266_status_t status;
	
	clock_setup_32_mhz();
	ht1632c_begin(HT1632_COMMON_16NMOS);
	ht1632c_setBrightness(0);
	ht1632c_clearScreen();
	//ht1632c_fillScreen();
	//adc_setup();
	uart_setup();
	pmic_setup();
	//i2c_setup();
	//btn_setup();
	//rtc_setup();
	jsmn_init(&p);
	
	stdout = stdin = &mystdout;
	
	puts("LED MATRIX Clock - By: Erlend Hestnes");
	
	//sd_card();
	
	//_delay_ms(5000);
	
	//si114x_reset(SI114X_ADDR);
	//si114x_init(SI114X_ADDR);
	
	
	//init_time();
	
	sei();
	
	esp8266_on();
	esp8266_setup_webserver();
	
	/*
	do {status = esp8266_setup(); } while (status != SUCCESS);
	do {status = esp8266_join_ap(SSID,PASS); } while (status != SUCCESS);
	status = esp8266_connect("hvilkenuke.no","hvilkenuke.no");
	esp8266_off();
	puts("GOT DATA:");
	esp8266_get_rx_buffer(&rx_buf);
	puts(rx_buf);
	*/
	//r = jsmn_parse(&p, &rx_buf, 100, tokens, 100);
	
	//print_token(tokens,&rx_buf,10);
	
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