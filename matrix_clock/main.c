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

int main(void) {
	
	clock_setup_32_mhz();
	ht1632c_setup(HT1632_COMMON_16NMOS);
	ht1632c_set_brightness(0);
	ht1632c_clear_screen();
	esp8266_off();
	
	uart_setup();
	stdout = stdin = &mystdout;
	puts("LED MATRIX Clock - By: Erlend Hestnes\r\n");
	
	pmic_setup();
	twi_setup(&TWIC);
	si114x_reset(SI114X_ADDR);
	twi_off();
	
	btn_setup();
	rtc_setup();
	rtc_set_time(30,0,0,0,2015);
	
	//ht1632_dummy();
	
	sei();
	
	SLEEP.CTRL |= SLEEP_MODE_PWR_SAVE;
		
	while (1) {
		//menu_state_machine();
		SLEEP.CTRL |= SLEEP_SEN_bm;
		asm("sleep");
	}
}
