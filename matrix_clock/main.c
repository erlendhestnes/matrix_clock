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
#include "drivers/eeprom/eeprom.h"

FATFS FatFs;		// FatFs work area needed for each volume
FIL Fil;			// File object needed for each open file
BYTE Buff[1024];	// Working buffer 2048
UINT bw;

typedef struct {
	uint16_t id;
	char *name;
	char *time; //maybe a time_t struct?
} env_variables_t;

#define TEST_BYTE_1 0x55
#define TEST_BYTE_2 0xAA

#define TEST_BYTE_ADDR_1 0x00
#define TEST_BYTE_ADDR_2 0x08

#define TEST_PAGE_ADDR_1    0  /* Page address always on a page boundary. */
#define TEST_PAGE_ADDR_2    2  /* Page address always on a page boundary. */
#define TEST_PAGE_ADDR_3    5  /* Page address always on a page boundary. */

/*! Testbuffer to write into EEPROM. */
uint8_t testBuffer[EEPROM_PAGESIZE] = {"Accessing Atmel AVR XMEGA EEPROM"};

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

static inline void delay_ms( int ms )
{
	for (int i = 0; i < ms; i++)
	{
		_delay_ms(1);
	}
}

int main(void) {
	
	bool test_ok = false;
	
	clock_setup_32_mhz();
	ht1632c_setup(HT1632_COMMON_16NMOS);
	ht1632c_set_brightness(0);
	//ht1632c_fill_screen();
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
	
	sei();
	/*
	SLEEP.CTRL |= SLEEP_MODE_PWR_SAVE;
	
	_delay_ms(100);
	
	EEPROM_FlushBuffer();
		
	EEPROM_EnableMapping();

	EEPROM_WaitForNVM();
	for (uint8_t i = 0; i < EEPROM_PAGESIZE; ++i) {
		EEPROM(TEST_PAGE_ADDR_3, i) = testBuffer[i];
	}

	EEPROM_WaitForNVM();
	for (uint8_t i = 0; i < EEPROM_PAGESIZE; ++i) {
		if (EEPROM(TEST_PAGE_ADDR_3, i) != testBuffer[i] ) {
			test_ok = false;
			ht1632c_fill_screen();
			break;
		}
	}
	*/
	while (1) { 
		
		//menu_state_machine();
		//SLEEP.CTRL |= SLEEP_SEN_bm;
		//asm("sleep");
	}
}
