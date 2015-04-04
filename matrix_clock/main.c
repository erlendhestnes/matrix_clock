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
#include "si114x.h"
#include "uart.h"
#include "fatfs/ff.h"
#include "fatfs/sound.h"
#include "fft/fft.h"
#include "power.h"
#include "esp8266.h"
#include "port.h"
#include "usb.h"

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
	ht1632c_setBrightness(0);
	//ht1632c_clearScreen();
	ht1632c_fillScreen();
	uart_setup();
	//btn_top_setup();
	pmic_setup();
	i2c_setup();
	//usb_setup();
	//rtc_setup();
	
	SLEEP.CTRL |= SLEEP_MODE_PWR_SAVE;
	
	stdout = stdin = &mystdout;
	
	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
	
	if (f_open(&Fil, "newfilea.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
	
		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
	
		f_close(&Fil);								/* Close the file */
	}

	//sei();
	
	//esp8266_on();
	//esp8266_init();
	
	//TCC1.CTRLA = TC_CLKSEL_DIV1024_gc;
	//TCC1.PER = 30000;
	//TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
	//init_time();
	//sei();
	
	button_t pressed;
	uint8_t i2c_data;
	
	uint8_t read_buffer[10];
	
	_delay_ms(5000);
	
	i2c_write(SI114X_ADDR,HW_KEY,0x17);
	
	_delay_ms(10);
	
	i2c_write(SI114X_ADDR,PS_LED21,0x05); //min current
	i2c_write(SI114X_ADDR,MEAS_RATE,0xB9); //wake up each 10ms
	i2c_write(SI114X_ADDR,PS_RATE,0x08); //make PS measurment on every wakeup
	i2c_write(SI114X_ADDR,ALS_RATE, 0x08); //make ALS measurment on every wakeup
	
	si114x_param_set(PARAM_CH_LIST,0x31); //Enable ALS and PS measurment
	si114x_param_set(PARAM_PSLED12_SELECT,0x01); //select LED1 for PS1
	si114x_param_set(PARAM_PS1_ADC_MUX, 0x03); //small photodiode
	si114x_param_set(PARAM_PS_ADC_COUNTER,0x10); //7ADC clock cycles
	si114x_param_set(PARAM_PS_ADC_GAIN,0x02);
	si114x_param_set(PARAM_PS_ADC_MISC,0x04); //Normal proximity mode and normal signal range

	i2c_write(SI114X_ADDR,COMMAND,0x00);
	i2c_read(SI114X_ADDR,RESPONSE,read_buffer);
	i2c_write(SI114X_ADDR,COMMAND,0b00001111); //Automatic ALS and PS measurement
	i2c_read(SI114X_ADDR,RESPONSE,read_buffer);
	
	while (1) {
		/*
		uint8_t i;

		pressed = btn_check_press();
		
		if (pressed) {
			ht1632c_clearScreen();
			ht1632c_drawChar(9,9,(pressed + '0'),1,1);
			ht1632c_writeScreen();
			//play_sound("test");
		}
		//SLEEP.CTRL |= SLEEP_SEN_bm;
		//asm("sleep");
		*/
		
		i2c_read(SI114X_ADDR,ALS_VIS_DATA0,read_buffer);
		//i2c_read(SI114X_ADDR,PS1_DATA0,read_buffer);
		//printf("PS: %d ,",read_buffer[0]);
		//printf("PS: %d ,",read_buffer[1]);
		//ht1632c_setBrightness(read_buffer[1]-10);
		printf("PS: %d \r\n",read_buffer[2]);
		_delay_ms(100);
	}
	
}
/*
ISR(PORTC_INT0_vect)
{
	fade_up();
	cnt++;
	_delay_ms(1500);
	fade_down();
}
*/
/*
ISR(TCC1_OVF_vect) {
	
	if (full == 1) {
		fhtDitInt(fht_array);
		//complexToReal(fht_array, 7);
		drawGraph64(fht_array, FHT_LEN/2);
		full = 2;
	}
}
*/

