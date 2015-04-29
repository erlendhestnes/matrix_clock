/*
 * matrix_clock.c
 *
 * Created: 10/9/2014 5:49:41 PM
 *  Author: Administrator
 */ 

#define F_CPU 32000000UL

#define RAND_MAX 255

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

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
#include "clock.h"

#include "si114x/User_defs.h"
#include "si114x/Si114x_functions.h"
#include "si114x/Si114x_handler.h"

static const unsigned char IMG_SPEAKER_A [] PROGMEM = {0b00011000, 0b00011000, 0b00111100, 0b01000010, 0b10100101, 0b00011000};
static const unsigned char IMG_SPEAKER_B [] PROGMEM = {0b00011000, 0b00011000, 0b00111100, 0b01000010, 0b10111101, 0b00000000};
#define IMG_SPEAKER_WIDTH 	 6
#define IMG_SPEAKER_HEIGHT 	 8

static const unsigned char IMG_HEART [] PROGMEM = {0b01110000, 0b11111000, 0b11111100, 0b11111110, 0b01111111, 0b11111110, 0b11111100, 0b11111000, 0b01110000};
#define IMG_HEART_WIDTH 	 9
#define IMG_HEART_HEIGHT 	 8

static const unsigned char IMG_MAIL [] PROGMEM = {0b11111111, 0b11000001, 0b10100001, 0b10010001, 0b10001001, 0b10000101, 0b10000101, 0b10001001, 0b10010001, 0b10100001, 0b11000001, 0b11111111};
#define IMG_MAIL_WIDTH 	12
#define IMG_MAIL_HEIGHT 8

static const unsigned char IMG_FB [] PROGMEM = {0b00111111, 0b01000000, 0b10000100, 0b10011111, 0b10100100, 0b10100000, 0b10000000, 0b10000000};
#define IMG_FB_WIDTH 	 7
#define IMG_FB_HEIGHT 	 8

static const unsigned char IMG_TWITTER [] PROGMEM = {0b01111110, 0b10000001, 0b10111001, 0b10010101, 0b10010101, 0b10000001, 0b01111110};
#define IMG_TWITTER_WIDTH 	 7
#define IMG_TWITTER_HEIGHT 	 8

static volatile uint16_t counter = 0;

FATFS FatFs;		// FatFs work area needed for each volume
FIL Fil;			// File object needed for each open file
BYTE Buff[1024];	// Working buffer 2048
UINT bw;

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

static inline void sd_card(void) {
	f_mount(&FatFs, "", 0);
	
	if (f_open(&Fil, "web.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		puts("Reading...");
		f_read(&Fil,Buff,101,&bw);
		f_close(&Fil);
	}
	puts("Read done:");
	puts(Buff);
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

void proximity_fade_demo(void) {
	
	uint8_t flip = 1;
	
	uint8_t reg0 = i2c_read_data(SI114X_ADDR,REG_PS1_DATA0);
	uint8_t reg1 = i2c_read_data(SI114X_ADDR,REG_PS1_DATA1);

	uint16_t reg01 = ((u16)reg1 << 8) | reg0;
	
	if (reg01 < 2200)
	{
		if (flip) {
			ht1632c_send_command(HT1632_LED_OFF);
			flip = 0;
		}
		} else {
		if (!flip) {
			ht1632c_send_command(HT1632_LED_ON);
			flip = 1;
		}
		ht1632_fade(reg01/400);
	}
}

int main(void) {

	//Note to self: There is a memory issue, try to reduce size of buffers

	time_t user_time;
	
	SI114X_IRQ_SAMPLE sensor_data;
	
	jsmn_parser p;
	jsmntok_t tokens[100];
	jsmnerr_t r;
	char rx_buf[200];
	
	esp8266_status_t status;
	
	clock_setup_32_mhz();
	ht1632c_setup(HT1632_COMMON_16NMOS);
	ht1632c_set_brightness(0);
	ht1632c_clear_screen();
	
	ht1632c_draw_char(2,9,'1',1,1);
	ht1632c_draw_char(9,9,'8',1,1);
	ht1632c_draw_char(2,0,'0',1,1);
	ht1632c_draw_char(9,0,'4',1,1);
	ht1632c_refresh_screen();
	
	//adc_setup();
	uart_setup();
	pmic_setup();
	twi_setup(&TWIC);

	//btn_setup();
	//rtc_setup();
	jsmn_init(&p);
	
	stdout = stdin = &mystdout;
	puts("LED MATRIX Clock - By: Erlend Hestnes\r\n");
	
	si114x_reset(SI114X_ADDR);
	//_delay_ms(1000);
	//si114x_init(SI114X_ADDR);
	
	sei();
	
	//sd_card();
	
	//esp8266_on();
	//esp8266_setup_webserver();
	
	/*
	do {status = esp8266_setup(); } while (status != SUCCESS);
	do {status = esp8266_join_ap(SSID,PASS); } while (status != SUCCESS);
	status = esp8266_connect("hvilkenuke.no","hvilkenuke.no");
	esp8266_off();
	puts("GOT DATA:");
	esp8266_get_rx_buffer(&rx_buf);
	puts(rx_buf);
	
	r = jsmn_parse(&p, &rx_buf, 100, tokens, 100);
	
	print_token(tokens,&rx_buf,10);
	*/
	while (1) {
		
		//esp8266_run_simple_webserver(Buff);
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
	}
}