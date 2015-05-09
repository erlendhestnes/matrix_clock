/*
 * menu.c
 *
 * Created: 5/1/2015 10:35:39 PM
 *  Author: Administrator
 */ 

#include "menu.h"
#include "drivers/rtc/rtc.h"
#include "drivers/port/port.h"
#include "drivers/ht1632c/ht1632c.h"
#include "drivers/esp8266/esp8266.h"
#include "drivers/sensors/si114x/Si114x_functions.h"
#include "json/jsmn.h"

static volatile uint16_t counter = 0;

void tcc_setup(void) {
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC1.PERL = 0x80;
	TCC1.PERH = 0x0C;
	TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
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

void menu_state_machine(void) {
	if (btn_status != NONE) {
		esp8266_status_t status;
		
		SI114X_IRQ_SAMPLE sensor_data;
		uint8_t flip = 1;
		
		jsmn_parser p;
		jsmntok_t tokens[50];
		jsmnerr_t r;
		char *json_buffer;
		
		switch(btn_status) {
			case BTN1:
				esp8266_on();
				ht1632c_scroll_print("STARTING TELNET SERVICE",false);
				status = esp8266_setup_webserver(true);
				if (status == ESP8266_SUCCESS) {
					esp8266_telnet_server();
				} else {
					ht1632c_scroll_print("ERROR: COULD NOT START TELNET SERVICE",false);
				}
				break;
			case BTN2:	
				si114x_setup();
				//ht1632c_scroll_print("STARTING GESTURE DEMO",false);
				
				while(1) {
					sensor_data.timestamp = counter;
					si114x_get_data(&sensor_data);
					si114x_process_samples(SI114X_ADDR,&sensor_data);

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
				}
				break;
			case BTN3:
				jsmn_init(&p);
				esp8266_on();
				do {status = esp8266_setup(); } while (status != ESP8266_SUCCESS);
				ht1632c_scroll_print("WIFI ON",false);
				do {status = esp8266_join_ap(SSID,PASS); } while (status != ESP8266_SUCCESS);
				ht1632c_scroll_print(SSID,false);
				status = esp8266_connect(DST_IP,ADDRESS,json_buffer);
				ht1632c_scroll_print("GOT DATA",false);
				esp8266_off();
				puts("GOT DATA:");
				puts(json_buffer);
			
				r = jsmn_parse(&p,json_buffer,strlen(json_buffer),tokens,100);
			
				print_token(tokens,json_buffer,3);
				print_token(tokens,json_buffer,4);
				print_token(tokens,json_buffer,5);
				break;
			case BTN4:

			break;
		}
		btn_status = NONE;
	}
}

//Used for SI114x Timestamp
ISR(TCC1_OVF_vect) {
	counter++;
}