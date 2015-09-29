/*
 * matrix_clock.c
 *
 * Created: 10/9/2014 5:49:41 PM
 *  Author: Administrator
 */ 

#include "drivers/ht1632c/ht1632c.h"
#include "drivers/rtc/rtc.h"
#include "drivers/sercom/twi.h"
#include "drivers/sercom/spi.h"
#include "drivers/sercom/uart.h"
#include "drivers/power/power.h"
#include "drivers/esp8266/esp8266.h"
#include "drivers/port/port.h"
#include "drivers/adc/adc.h"
#include "drivers/clock/clock.h"

#include "fatfs/ff.h"
#include "fatfs/sound.h"

#include "drivers/sensors/si114x/User_defs.h"
#include "drivers/sensors/si114x/Si114x_functions.h"
#include "drivers/sensors/si114x/Si114x_handler.h"
#include "drivers/eeprom/eeprom.h"

#include "modules/display/display.h"
#include "modules/time/time_functions.h"
#include "modules/menu/menu.h"

#include "json/jsmn.h"
#include "json/json_functions.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_put_char,uart_get_char,_FDEV_SETUP_WRITE);

void pmic_setup(void) 
{
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
}

void play_sound(void) 
{
	FATFS FatFs;		// FatFs work area needed for each volume
	FIL Fil;			// File object needed for each open file
	BYTE Buff[1024];	// Working buffer 1024
	UINT bw;
	
	while (1)
	{
		f_mount(&FatFs, "", 0);
		
		BYTE res;
		res = f_open(&Fil, "cold3.wav", FA_READ);
		if (!res) {
			load_wav(&Fil, "**** WAV PLAYER ****", Buff, sizeof Buff);
			f_close(&Fil);
		}
	}
}

int main(void) 
{	
	//Structures
	SI114X_IRQ_SAMPLE sensor_data;
	
	//System
	clock_setup_32_mhz();
	
	//LED Matrix init
	display_setup();
	
	//Init env variables
	if(1)
	//if (!EEPROM_ReadEnv())
	{
		menu_set_env_variables();
	}
	//Guard
	env_var.menu_id = 0;
	
	//WiFi off
	esp8266_off();
	
	//Debug interface
	uart_setup();
	stdout = stdin = &mystdout;

#ifdef DEBUG_ON
	puts("LED MATRIX Clock - By: Erlend Hestnes\r\n");
#endif

	//Enable interrupts
	pmic_setup();
	btn_setup(false);
	
	twi_setup(&TWIC);

	SLEEP.CTRL |= SLEEP_MODE_PWR_SAVE;
	
	sei();
	
	//Calculate baseline for Si114x
	si114x_baseline_calibration(&sensor_data);

	//Turn on proximity channel 1 with ISR and threshold
	si114x_setup_ps1_only();
	
	//Turn on RTC
	rtc_enable_time_render();
	ht1632c_refresh_screen();
	rtc_setup();
	
	EEPROM_WriteEnv();
	
	//This should flip MOSI and SCK
	//PORTC.REMAP |= PORT_SPI_bm;
	
	while (1) { 
		if (si114x_status == 4)
		{
			uint16_t timeout_ms;
			bool timeout = false;
			
			btn_setup(false);
			btn_si114x_disable_interrupt();
			
			display_fade(MAX_BRIGHTNESS);
#ifdef DEBUG_ON
			puts("Enter gesture mode!");
#endif
			si114x_setup();
			
			//To avoid first menu item of being selected
			_delay_ms(500);
			
			while(!timeout) {
				si114x_get_data(&sensor_data);
				si114x_process_samples(SI114X_ADDR,&sensor_data);
				
				menu_state_machine(&sensor_data);
				
				if (sensor_data.ps1 < PROXIMITY_THRESHOLD) {
					if (timeout_ms++ > 15000) {
						timeout = true;
					}
				} else {
					timeout_ms = 0;
				}
			}
		    EEPROM_WriteEnv();
			
			timeout_ms = 0;
#ifdef DEBUG_ON
			puts("Timeout! \n");
#endif
			if (env_var.menu_id != 0) {
				display_slide_out_to_bottom();
				rtc_enable_time_render();
				display_slide_in_from_top();
			}
			display_fade(env_var.brightness);
			btn_setup(true);
			btn_si114x_enable_interrupt();
			si114x_setup_ps1_only();
			si114x_status = 0;
		} else if (si114x_status == 2) {
			//Dim light by using the light sensor
			puts("Somebody turned off the lights!");
		}
		
		SLEEP.CTRL |= SLEEP_SEN_bm;
		asm("sleep");
	}
}