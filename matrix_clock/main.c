/*
 * matrix_clock.c
 *
 * Created: 10/9/2014 5:49:41 PM
 *  Author: Administrator
 */ 

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

void uart_put_char(char c);
static FILE mystdout = FDEV_SETUP_STREAM(uart_put_char,uart_get_char,_FDEV_SETUP_WRITE);

void pmic_setup(void) 
{
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
}

void play_sound(char *name) {
	
	FATFS FatFs;		// FatFs work area needed for each volume
	FIL Fil;			// File object needed for each open file
	BYTE Buff[1024];	// Working buffer 2048
	UINT bw;
	
	while (1)
	{
		f_mount(&FatFs, "", 0);

		if (f_open(&Fil, "newfilea.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
			
			f_write(&Fil, "It works!\r\n", 11, &bw);
			
			f_close(&Fil);
		}
		
		BYTE res;
		res = f_open(&Fil, "cold3.wav", FA_READ);
		if (!res) {
			load_wav(&Fil, "**** WAV PLAYER ****", Buff, sizeof Buff);
			f_close(&Fil);
		}
	}
}

/*
static inline void sd_card(void) {
	f_mount(&FatFs, "", 0);
	
	char line[200];
	
	if (f_open(&Fil, "website2.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		puts("Reading...");
		//f_read(&Fil,Buff,256,&bw);
		while(f_gets(line,sizeof(line),&Fil))
			printf("%s",line);
		
		f_close(&Fil);
	}
	puts("Read done:");

}
*/


void print_token(jsmntok_t *tokens, char *js, uint8_t i) {
	int len;
	jsmntok_t key;

	key = tokens[i];
	len = key.end - key.start;
	char keyString[ len+1 ];
	memcpy( keyString, &js[ key.start ], len );
	keyString[ len ] = '\0';
	//printf( "Key[%d]: %s\n", i, keyString );
	printf("token[%d]: %s \r\n",i,keyString);
}

esp8266_status_t wifi_status = 0;

int main(void) {
	
	bool test_ok = false;
	
	//Structures
	SI114X_IRQ_SAMPLE sensor_data;
	
	//System
	clock_setup_32_mhz();
	
	//LED Matrix init
	ht1632c_setup(HT1632_COMMON_16NMOS);
	ht1632c_set_brightness(1);
	ht1632c_clear_screen();
	ht1632c_send_command(HT1632_LED_ON);
	
	//Init env variables
	if(1)
	//if (!EEPROM_ReadEnv())
	{
		EEPROM_EraseAll();
		
		strncpy(env_var.name,CLOCK_NAME,sizeof(env_var.name));
		env_var.id = CLOCK_ID;
		env_var.temperature[0] = '0';
		env_var.brightness = 1;
		env_var.menu_id = 0;
		strncpy(env_var.wifi_pswd,PASS,sizeof(env_var.wifi_pswd));
		strncpy(env_var.wifi_ssid,SSID,sizeof(env_var.wifi_ssid));
			
		env_var.time.seconds = 30;
		env_var.time.minutes = 59;
		env_var.time.hours = 23;
		env_var.time.day = 31;
		env_var.time.weekday = Sunday;
		env_var.time.week = 52;
		env_var.time.month = December;
		env_var.time.year = 2015;
		
		env_var.alarm.hours = 0;
		env_var.alarm.minutes = 0;
	}
	
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
	
	
	//play_sound("care");
	
	esp8266_on();
	//esp8266_setup();
	esp8266_setup_webserver(false, true);
	while (1)
	{
		esp8266_run_simple_webserver();
	}
	
	
	//sd_card();
	
	//This should flip MOSI and SCK
	//PORTC.REMAP |= PORT_SPI_bm;
	
	/*
	ht1632c_clear_screen();
	
	TCC0.CNT = 0;
	TCC0.PER = 3125;
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	
	TCC0.CCA = 3125;
	TCC0.INTCTRLB |= TC_CCAINTLVL_LO_gc;
	TCC0.CTRLB |= TC0_CCAEN_bm;
	
	TCC0.CCB = 781;
	TCC0.INTCTRLB |= TC_CCBINTLVL_LO_gc;
	TCC0.CTRLB |= TC0_CCBEN_bm;
	
	while(1) {
		
	}
	*/
	while (1) { 
		if (si114x_status == 4)
		{
			uint16_t timeout_ms;
			bool timeout = false;
			
			btn_setup(false);
			btn_si114x_disable_interrupt();
			
			ht1632c_fade(MAX_BRIGHTNESS);
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
				ht1632c_slide_out_to_bottom();
				rtc_enable_time_render();
				ht1632c_slide_in_from_top();
			}
			ht1632c_fade(env_var.brightness);
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