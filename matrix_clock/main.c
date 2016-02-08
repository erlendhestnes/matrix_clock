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
#include "drivers/port/port.h"
#include "drivers/adc/adc.h"
#include "drivers/clock/clock.h"
#include "drivers/eeprom/eeprom.h"
#include "drivers/esp8266/esp8266.h"
#include "drivers/timer/timer.h"
#include "drivers/power/power.h"

#include "drivers/sensors/si114x/User_defs.h"
#include "drivers/sensors/si114x/Si114x_functions.h"
#include "drivers/sensors/si114x/Si114x_handler.h"

#include "modules/display/display.h"
#include "modules/time_functions/time_functions.h"
#include "modules/menu/menu.h"
#include "modules/json/jsmn.h"
#include "modules/json/json_functions.h"
#include "modules/fatfs/ff.h"
#include "modules/fatfs/sound.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_put_char,uart_get_char,_FDEV_SETUP_WRITE);

void pmic_setup(void) 
{
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
}

void ultra_power_saving_mode_test(void) 
{	
	//maybe add pullups
	si114x_reset((HANDLE)SI114X_ADDR);
	//si114x_setup_ps1();
	twi_off();
	uart_disable();
	rtc_enable_time_render();
	display_refresh_screen();
	display_fade(3);
	display_fade(0);
	ht1632c_power_down();
	SLEEP.CTRL |= SLEEP_MODE_PWR_SAVE;
	SLEEP.CTRL |= SLEEP_SEN_bm;
	asm("sleep");
	while(1);
}

int main(void) 
{	
	lowpower_init();
	
	//Structures
	SI114X_IRQ_SAMPLE sensor_data;
	
	//System
	clock_setup_32_mhz_pll();
	display_setup();
	esp8266_off();
	spi_disable();
	
	//Init env variables
	if(1)
	//if (!EEPROM_ReadEnv())
	{
		menu_set_env_variables();
	} 
	
	//Debug interface
	uart_setup();
	stdout = stdin = &mystdout;

#ifdef DEBUG_ON
	puts("SMART Clock - By: Erlend Hestnes\r\n");
#endif

	//Enable interrupts
	pmic_setup();
	btn_setup(true);
	
	twi_setup(&TWIC);

	SLEEP.CTRL |= SLEEP_MODE_PWR_SAVE;
	
	sei();
	
	/*
	//Ultra low power test
	ultra_power_saving_mode_test();
	while(1);
	*/
	
	si114x_baseline_calibration(&sensor_data);
	
	//Turn on proximity channel 1 with ISR and threshold
	si114x_setup_ps1();
	
	/*
	//Si114x test
	while(1) {
		si114x_get_data(&sensor_data);
		printf("PS1: %d \r\n",sensor_data.ps1);
		//_delay_ms(500);
	}
	*/
	
	//Turn on RTC
	display_fade(0);
	rtc_enable_time_render();
	display_refresh_screen();
	rtc_setup();
	
	EEPROM_WriteEnv();
	
	//This should flip MOSI and SCK, if DMA should be used at some point...
	//PORTC.REMAP |= PORT_SPI_bm;
	
	bool display = true;
	
	while (1) { 	
		if (si114x_status == PS1_INT) {
			uint16_t timeout_ms = 0;
			bool timeout = false;
			env.menu_id = 0;
			
			btn_setup(false);
			btn_disable_si114x_interrupt();
			
			display_fade(MAX_BRIGHTNESS);
#ifdef DEBUG_ON
			puts("DEBUG: Entered gesture mode.");
#endif
			si114x_setup();
			_delay_ms(500);
			
			while(!timeout) {
				si114x_get_data(&sensor_data);
				si114x_process_samples((HANDLE)SI114X_ADDR,&sensor_data);
				
				menu_state_machine(&sensor_data);
				
				if (sensor_data.ps1 < PROXIMITY_THRESHOLD2) {
					if (timeout_ms++ > MENU_TIMEOUT) {
						timeout = true;
					}
				} else {
					timeout_ms = 0;
				}
			}
			//Should not write too often to EEPROM
		    EEPROM_WriteEnv();
			timeout_ms = 0;
#ifdef DEBUG_ON
			puts("DEBUG: Timeout! Leaving gesture mode.");
#endif
			if (env.menu_id != 0) {
				display_slide_out_to_bottom();
				rtc_enable_time_render();
				display_slide_in_from_top();
			}
			display_fade(env.brightness);
			btn_setup(true);
			btn_si114x_enable_interrupt();
			si114x_setup_ps1();
		} else if (si114x_status == ALS_INT_2) {
			//Dim light by using the light sensor
#ifdef DEBUG_ON
			puts("DEBUG: Somebody turned off the lights!");
#endif
		} else if (btn_status == (BTN1 | BTN4)) {
			//Calculate baseline for Si114x
			display_slide_out_to_bottom();
			si114x_baseline_calibration(&sensor_data);
			rtc_enable_time_render();
			display_slide_in_from_top();
		} else if (btn_status == BTN4) {
			if (display) {
				display_off();
				_delay_ms(1000);
			} else {
				display_on();
				_delay_ms(1000);
			}
			btn_status = NO_BTN;
			display ^= true;
		}
		si114x_status = 0;
		
		SLEEP.CTRL |= SLEEP_SEN_bm;
		asm("sleep");
	}
}