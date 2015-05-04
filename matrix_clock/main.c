/*
 * matrix_clock.c
 *
 * Created: 10/9/2014 5:49:41 PM
 *  Author: Administrator
 */ 

#define F_CPU 32000000UL

#define RAND_MAX 255

#include "drivers/ht1632c/ht1632c.h"
#include "drivers/rtc/rtc.h"
#include "drivers/sercom/sercom.h"
#include "uart.h"
#include "fatfs/ff.h"
#include "fatfs/sound.h"
#include "drivers/power/power.h"
#include "drivers/esp8266/esp8266.h"
#include "drivers/port/port.h"
#include "drivers/adc/adc.h"
#include "json/jsmn.h"
#include "drivers/clock/clock.h"

#include "drivers/sensors/si114x/User_defs.h"
#include "drivers/sensors/si114x/Si114x_functions.h"
#include "drivers/sensors/si114x/Si114x_handler.h"

static volatile uint16_t counter = 0;

FATFS FatFs;		// FatFs work area needed for each volume
FIL Fil;			// File object needed for each open file
BYTE Buff[2048];	// Working buffer 2048
UINT bw;

typedef struct {
	uint16_t id;
	char *name;
	char *time; //maybe a time_t struct?
} env_variables_t;

void uart_put_char(char c);
static FILE mystdout = FDEV_SETUP_STREAM(uart_put_char,uart_get_char,_FDEV_SETUP_WRITE);

void pmic_setup(void) {
	
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
}

void play_sound(char *name) {
	
	BYTE res;
	res = f_open(&Fil, "cold3.wav", FA_READ);
	if (!res) {
		load_wav(&Fil, "**** WAV PLAYER ****", Buff, sizeof Buff);
		f_close(&Fil);
	}	
}

static inline void sd_card(void) {
	f_mount(&FatFs, "", 0);
	
	if (f_open(&Fil, "website2.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		puts("Reading...");
		f_read(&Fil,Buff,1024,&bw);
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
	SI114X_IRQ_SAMPLE sensor_data;
	
	jsmn_parser p;
	//jsmntok_t tokens[100];
	jsmnerr_t r;
	
	char *cmd;
	//char json_buffer[RX_BUFFER];
	
	esp8266_status_t status;
	
	clock_setup_32_mhz();
	ht1632c_setup(HT1632_COMMON_16NMOS);
	ht1632c_set_brightness(0);
	ht1632c_clear_screen();
	
	uart_setup();
	pmic_setup();
	twi_setup(&TWIC);
	si114x_reset(SI114X_ADDR);
	//si114x_setup();
	//tcc_setup();
	btn_setup();
	//rtc_setup();
	//rtc_init_time();
	jsmn_init(&p);
	
	stdout = stdin = &mystdout;
	puts("LED MATRIX Clock - By: Erlend Hestnes\r\n");

	//rtc_set_time(18,50,0);
	
	sd_card();
	
	sei();
	
	esp8266_off();
	//esp8266_on();
	//esp8266_setup_webserver();
	
	/*
	do {status = esp8266_setup(); } while (status != ESP8266_SUCCESS);
	ht1632c_scroll_print("Wifi on",0,0);
	do {status = esp8266_join_ap(SSID,PASS); } while (status != ESP8266_SUCCESS);
	ht1632c_scroll_print(SSID,0,0);
	status = esp8266_connect(DST_IP,ADDRESS,json_buffer);
	ht1632c_scroll_print("Got Data",0,0);
	esp8266_off();
	puts("GOT DATA:");
	puts(json_buffer);
	
	r = jsmn_parse(&p,json_buffer,strlen(json_buffer),tokens,100);
	
	print_token(tokens,json_buffer,3);
	print_token(tokens,json_buffer,4);
	print_token(tokens,json_buffer,5);
	
	//uint8_t *data;
	//uint8_t flip = 1;
	
	*/
	play_sound("whatever!");
	
	while (1) {
		
		//ht1632c_scroll_print("16:04",4,4);
		//esp8266_run_simple_webserver(Buff);
		
		/*
		sensor_data.timestamp = counter;
		si114x_get_data(&sensor_data);
		si114x_process_samples(SI114X_ADDR,&sensor_data); 
		*/
		
		/*
		if (sensor_data.ps1 < 1600)
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
			ht1632_fade(sensor_data.ps1/300);
		}
		*/
		//rtc_update_display_alt();
	}
}

//Used for SI114x Timestamp
ISR(TCC1_OVF_vect) {
	counter++;
}
