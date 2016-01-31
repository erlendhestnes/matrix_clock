/*
 * esp8266.c
 *
 * Created: 1/18/2015 1:42:28 PM
 *  Author: Administrator
 */ 
#include "esp8266.h"
#include "../ht1632c/ht1632c.h"
#include "../eeprom/eeprom.h"
#include "../rtc/rtc.h"
#include "../port/port.h"
#include "../sercom/uart.h"

#include "../../modules/fatfs/ff.h"
#include "../../modules/display/display.h"

#include <stdlib.h>

volatile bool json_found = false;
volatile bool wdt_triggered = false;
volatile char link_channel = '0';

volatile static uint16_t rx_ptr = 0;
static char rx_buffer[RX_BUFFER];

char ip_address[19];

esp8266_status_t status;

static inline void esp8266_send_cmd(char *str, uint16_t timeout_ms) 
{
	status = ESP8266_NONE;
	uart_write_str(str);
	_delay_ms(timeout_ms);
}

void esp8266_on(void) 
{
	PORTD.DIRSET = CH_EN;
	PORTD.OUTSET = CH_EN;
	_delay_ms(500);
}

void esp8266_off(void) 
{
	PORTD.OUTCLR = CH_EN;
}

void esp8266_reset(void) 
{
	PORTD.OUTSET = ESP_RST;
	_delay_ms(500);
	PORTD.OUTCLR = ESP_RST;
	_delay_ms(500);
}

esp8266_status_t esp8266_setup(void) 
{	
	//Reset module
	esp8266_send_cmd("AT+RST",5000);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	/*
	esp8266_send_cmd("AT+IPR=115200",5000);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	*/
	
	//Show firmware info
	//esp8266_send_cmd("AT+GMR",1000);
	//if (status != ESP8266_SUCCESS) {
	//	return status;
	//}
	
	//Set Data Mode
	esp8266_send_cmd("AT+CIPMODE=0",1000);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Single connection mode
	esp8266_send_cmd("AT+CIPMUX=0",250);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Select STA mode
	esp8266_send_cmd("AT+CWMODE=1",250);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	return ESP8266_SUCCESS;
}

void esp8266_list_ap(void) 
{
	esp8266_send_cmd("AT+CWLAP",4000);
}

void esp8266_update(void) 
{
	esp8266_send_cmd("AT+CIPUPDATE",10000);
}

esp8266_status_t esp8266_join_ap(char *ssid, char *pass) 
{	
	uint16_t timeout = 300;
	uint16_t cnt = 0;
	char cmd[100];
	
	strcpy(cmd,"AT+CWJAP=\"");
	strcat(cmd,ssid);
	strcat(cmd,"\",\"");
	strcat(cmd,pass);
	strcat(cmd,"\"");
	esp8266_send_cmd(cmd,250);
	
	while (status != ESP8266_SUCCESS)
	{
		_delay_ms(100);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			return status;
		}
	}

	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_get_json(char *host, char *addr, char *json, uint8_t json_length) 
{	
	uint16_t timeout = 300;
	uint16_t cnt = 0;
	//Try to make this dynamic?
	char cmd[150];
	
	//Set up TCP connection to host
	strcpy(cmd,"AT+CIPSTART=\"TCP\",\"");
	strcat(cmd,host);
	strcat(cmd,"\",80");
	esp8266_send_cmd(cmd,250);
	
	if ((status != ESP8266_SUCCESS) || (status != ESP8266_CONNECT)) {
		return status;
	}
	
	//Count number of bytes to send
	char *number_of_bytes;
	itoa(strlen(addr) + 25, number_of_bytes, 10);
	strcpy(cmd, "AT+CIPSEND=");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here...
	esp8266_send_cmd(cmd,50);
	
	//Request data by sending a GET
	strcpy(cmd,"GET ");
	strcat(cmd,addr);
	strcat(cmd," HTTP/1.0\r\n");
	esp8266_send_cmd(cmd,50);
	
	while (status != ESP8266_SUCCESS)
	{
		esp8266_send_cmd("",100);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			break;
		}
	}
	
	//Waiting for all the data
	while (status != ESP8266_CLOSED)
	{
		_delay_ms(100);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			break;
		}
	}
	
	if (json_found) {
		uint16_t start_addr = (uint16_t)rx_buffer;
		uint16_t end_addr = (uint16_t)strrchr(rx_buffer,'}');
		uint16_t len = (end_addr-start_addr)+1;
		if (len > json_length) {
			return ESP8266_ERROR;
		}
		memset(json,0,json_length);
		strncpy(json,rx_buffer,len);
		memset(rx_buffer,0,RX_BUFFER);
		json_found = false;
		rx_ptr = 0;	
	}
	
	esp8266_send_cmd("AT+CIPCLOSE",50);
	
	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_setup_webserver(bool sta, bool ap) 
{	
	//Reset module
	esp8266_reset();
	
	//Set Data Mode
	esp8266_send_cmd("AT+CIPMODE=0",250);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Select mode
	if (sta) {
		esp8266_send_cmd("AT+CWMODE=1",250);
	} else if (ap) {
		esp8266_send_cmd("AT+CWMODE=2",250);
	} else if (ap && sta) {
		esp8266_send_cmd("AT+CWMODE=3",250);
	} else {
		return ESP8266_ERROR;
	}
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	if (sta || (sta && ap)) {
		esp8266_join_ap(env_var.wifi_ssid,env_var.wifi_pswd);
	}
	
	//List ip addresses
	esp8266_send_cmd("AT+CIFSR", 250);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Show IP address to user
#ifdef SHOW_MANUAL
	display_print_scrolling_text(ip_address,false);
#endif
	//Configure multiple connections
	esp8266_send_cmd("AT+CIPMUX=1",250);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Start server
	esp8266_send_cmd("AT+CIPSERVER=1,80",250);	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	return ESP8266_SUCCESS;
}

static inline void at_cipsend(char channel, char *str) 
{
	char number_of_bytes[5];
	char cmd[50];
	uint16_t timeout = 0;
	
	itoa(strlen(str),number_of_bytes,10);
	if (channel == '0') {
		strcpy(cmd, "AT+CIPSEND=0,");
	} else {
		strcpy(cmd, "AT+CIPSEND=1,");
	}
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r");

	esp8266_send_cmd(cmd,100);
	while((rx_buffer[0] != '>') && (timeout++ < 50)) {
		_delay_ms(100);
	}
	printf("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n%s\r\n\r\n",str);
	_delay_ms(100);
}

static inline void at_cipsend_from_sd_card(void) 
{
	char cmd[50];
	
	FATFS FatFs;		// FatFs work area needed for each volume
	FIL Fil;			// File object needed for each open file
	UINT br;
	
	f_mount(&FatFs, "", 0);
	
	char line[1024];
	memset(line,0,1024);
	
	if (f_open(&Fil, "comp.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		uint32_t filesize = f_size(&Fil);
		do 
		{ 
			f_read(&Fil,line,1024,&br);
			esp8266_send_cmd("AT+CIPSEND=0,1024\r",25);
			printf("%s",line);
			_delay_ms(25);
		} while (f_eof(&Fil) == 0);
		
		f_close(&Fil);
	} else {
		uart_write_str("Could not access sd card...");
	}
	
	if (f_open(&Fil, "zepto.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		uint32_t filesize = f_size(&Fil);
		do { 
			f_read(&Fil,line,1024,&br);
			esp8266_send_cmd("AT+CIPSEND=0,1024\r",0);
			while(status != ESP8266_SUCCESS);
			status = ESP8266_NONE;
			printf("%s",line);
			while(status != ESP8266_SUCCESS);
		} while (f_eof(&Fil) == 0);
		
		f_close(&Fil);
	} else {
		uart_write_str("Could not access sd card...");
	}
}

esp8266_status_t esp8266_configure_ssid_and_password(void) 
{
	if (status == ESP8266_GET_REQ) {
		
		at_cipsend(link_channel,"<!DOCTYPE html>\
		<html>\
		<body>\
		<p>Smart Clock - Configuration</p>\
		<form method=\"post\" target=\"_self\">\
		SSID: <input type=\"text\" name=\"my_ssid\"><br>\
		PASS: <input type=\"password\" name=\"my_password\"><br>\
		ZIP-Code:<input type=\"text\" name=\"zip\"><br>\
		NAME:<input type=\"text\" name=\"name\"><br>\
		<input type=\"submit\" value=\"Submit\">\
		</form>\
		<p>Input network name (SSID) and password for your router. Then click submit.</p>\
		</body>\
		</html>");
		
		_delay_ms(1000);
		
		if (link_channel == '0') {
			esp8266_send_cmd("AT+CIPCLOSE=0\r", 100);
		} else {
			esp8266_send_cmd("AT+CIPCLOSE=1\r", 100);	
		}
		
	} else if(status == ESP8266_POST_REQ) {
		uint16_t timeout = 0;
		
		while(timeout++ < 30) {
			_delay_ms(100);
			if (strstr(rx_buffer,"my_password") != NULL) {
				
				uint8_t ssid_len = sizeof(env_var.wifi_ssid);
				uint8_t pass_len = sizeof(env_var.wifi_pswd);
				
				memset(env_var.wifi_ssid,0,ssid_len);
				memset(env_var.wifi_pswd,0,pass_len);
				
				uint16_t len = strlen(rx_buffer);
				uint8_t n = 0;
				
				for (uint16_t i = 0; i < len; i++) {
					if(rx_buffer[i] == '=') {
						n++;
						char temp[20];
						uint8_t j = 0;
						memset(temp,0,20);
						while ((rx_buffer[++i] != '&') && (i != len)) {
							temp[j++] = rx_buffer[i];
						}
						if (n == 1) {
							if (strlen(temp) < ssid_len)
								strcpy(env_var.wifi_ssid,temp);
						} else if (n == 2) {
							if (strlen(temp) < pass_len)
								strcpy(env_var.wifi_pswd,temp);
							break;
						}
					}
				}
				
				if (link_channel == '0') {
					esp8266_send_cmd("AT+CIPCLOSE=0\r", 100);
				} else {
					esp8266_send_cmd("AT+CIPCLOSE=1\r", 100);
				}
				
				esp8266_off();
				_delay_ms(1000);
				esp8266_on();
				status = esp8266_setup();
				if (status != ESP8266_SUCCESS) {
#ifdef SHOW_MANUAL
					display_print_scrolling_text("COULD NOT JOIN AP",false);
#endif
					return ESP8266_TIMEOUT;
				}
				
				status = esp8266_join_ap(env_var.wifi_ssid,env_var.wifi_pswd);
				if (status != ESP8266_SUCCESS) {
#ifdef SHOW_MANUAL
					display_print_scrolling_text("COULD NOT JOIN AP",false);
#endif
					return ESP8266_TIMEOUT;
				}
#ifdef SHOW_MANUAL
				display_print_scrolling_text("WIFI CONFIGURED",false);
#endif
				return ESP8266_TIMEOUT;
			}	
		}
	}
	return ESP8266_SUCCESS;
}

ISR(USARTD0_RXC_vect) 
{
	
	char rx_temp = USARTD0.DATA;
	
	if (rx_temp == '\n') {
		if (strstr(rx_buffer,"OK") || strstr(rx_buffer,"ready")) {
			status = ESP8266_SUCCESS;
		} else if (strstr(rx_buffer,"ERROR") || strstr(rx_buffer,"FAIL")) {
			status = ESP8266_ERROR;
		} else if (strstr(rx_buffer,"CONNECT")) {
			status = ESP8266_CONNECT;
		} else if (strstr(rx_buffer,"CLOSED")) {
			status = ESP8266_CLOSED;
		} else if (strstr(rx_buffer,"GET")) {
			link_channel = rx_buffer[5];
			status = ESP8266_GET_REQ;
		} else if (strstr(rx_buffer,"POST")) {
			status = ESP8266_POST_REQ;
		} else if (strstr(rx_buffer,"192")) {
			strncpy(ip_address,strchr(rx_buffer,'\"'),19);
		} 
		if (strstr(rx_buffer,"{")) {
			json_found = true;	
		} else {
			//Flush buffer
			rx_ptr = 0;
			memset(rx_buffer, 0, RX_BUFFER);	
		}
	} else {
		rx_buffer[rx_ptr++] = rx_temp;
		
		//Buffer overflow guard
		if (rx_ptr == RX_BUFFER) {
			rx_ptr = 0;
		}
	}
}