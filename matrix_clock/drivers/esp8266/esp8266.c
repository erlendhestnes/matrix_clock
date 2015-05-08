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

volatile bool json_found = false;
volatile bool got_reply = false;

volatile static uint16_t rx_ptr = 0;

static char rx_buffer[RX_BUFFER];

char ip_address[20];
char telnet_cmd[50];

esp8266_status_t status;

static inline void esp8266_send_cmd(char *str, uint16_t timeout_ms) {
	status = ESP8266_NONE;
	uart_write_str(str);
	_delay_ms(timeout_ms);
}

void esp8266_on(void) {
	PORTD.DIRSET = CH_EN;
	PORTD.OUTSET = CH_EN;
	_delay_ms(3000);
}

void esp8266_off(void) {
	PORTD.OUTCLR = CH_EN;
}

void esp8266_reset(void) {
	PORTD.OUTSET = ESP_RST;
	_delay_ms(100);
	PORTD.OUTCLR = ESP_RST;
	_delay_ms(100);
}

esp8266_status_t esp8266_setup(void) {
    
	//Reset module
	esp8266_send_cmd("AT+RST",4000);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Set Data Mode
	esp8266_send_cmd("AT+CIPMODE=0",500);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Single connection mode
	esp8266_send_cmd("AT+CIPMUX=0",500);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Select mode
	esp8266_send_cmd("AT+CWMODE=1",500);
	
	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_join_ap(char *ssid, char *pass) {
	
	uint8_t timeout = 60;
	uint16_t cnt = 0;
	char cmd[100];
	
	strcpy(cmd,"AT+CWJAP=\"");
	strcat(cmd,ssid);
	strcat(cmd,"\",\"");
	strcat(cmd,pass);
	strcat(cmd,"\"");
	esp8266_send_cmd(cmd,500);
	
	while (status != ESP8266_SUCCESS)
	{
		_delay_ms(1000);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			return status;
		}
	}
	
	//Try to reduce delay here...
	esp8266_send_cmd("AT+CWJAP?",500);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}

	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_connect(char *host, char *addr, char *json) {
	
	uint8_t timeout = 60;
	uint16_t cnt = 0;
	//Try to make this dynamic?
	char cmd[100];
	
	//Set up TCP connection to host
	strcpy(cmd,"AT+CIPSTART=\"TCP\",\"");
	strcat(cmd,host);
	strcat(cmd,"\",80");
	esp8266_send_cmd(cmd,1000);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Count number of bytes to send
	char *number_of_bytes;
	itoa(strlen(addr)+25, number_of_bytes, 10);
	strcpy(cmd, "AT+CIPSEND=");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here...
	esp8266_send_cmd(cmd,500);
	
	//Request data by sending a GET
	strcpy(cmd,"GET ");
	strcat(cmd,addr);
	strcat(cmd," HTTP/1.0\r\n");
	esp8266_send_cmd(cmd,500);
	
	while (status != ESP8266_SUCCESS)
	{
		esp8266_send_cmd("",1000);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			break;
		}
	}
	
	//Waiting for all the data
	while (status != ESP8266_CLOSED)
	{
		_delay_ms(1000);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			break;
		}
	}
	
	if (json_found) {
		strncpy(json,rx_buffer,strlen(rx_buffer)-15);
		json_found = false;
		rx_ptr = 0;
		memset(rx_buffer, 0, RX_BUFFER);
	}
	
	esp8266_send_cmd("AT+CIPCLOSE",1000);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_setup_webserver(bool telnet) {
	
	//Reset module
	esp8266_send_cmd("AT+RST\r",3000);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Select mode
	if (telnet) {
		esp8266_send_cmd("AT+CWMODE=3",500);
	} else {
		esp8266_send_cmd("AT+CWMODE=1",500);
	}
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Join access point
	esp8266_join_ap(SSID,PASS);
	
	//List ip addresses
	esp8266_send_cmd("AT+CIFSR", 500);
	
	//Show ip address to user
	// ht1632c_scroll_print(ip_address,1,1);
	
	//Configure multiple connections
	esp8266_send_cmd("AT+CIPMUX=1",500);
	
	//Start server
	if (telnet) {
		esp8266_send_cmd("AT+CIPSERVER=1,8888",500);
		// ht1632c_scroll_print("Telnet on",1,1);	
	} else {
		esp8266_send_cmd("AT+CIPSERVER=1,80",500);	
	}
	
	return ESP8266_SUCCESS;
}

static inline void at_cipsend(char *str) {
	char number_of_bytes[5];
	char cmd[50];
	
	got_reply = false;

	itoa(strlen(str),number_of_bytes,10);
	strcpy(cmd, "AT+CIPSEND=0,");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here

	esp8266_send_cmd(cmd,50);
	esp8266_send_cmd(str,50);
}

esp8266_status_t esp8266_telnet_server(void) {
	
	if (status == ESP8266_CONNECT) {
		status = ESP8266_NONE;
		memset(telnet_cmd, 0, 50);
		at_cipsend("Welcome to the LED Matrix Clock telnet interface\r\n");
		at_cipsend("Type \"man\" to view all available commands\r\n>");
	} else if (strstr(telnet_cmd,"led")) {
		if (strstr(telnet_cmd,"on")) {
			memset(telnet_cmd, 0, 50);
			ht1632c_fill_screen();
			at_cipsend("Turning on LEDs\r\n>");
		} else if (strstr(telnet_cmd,"off")) {
			ht1632c_clear_screen();
			memset(telnet_cmd, 0, 50);
			at_cipsend("Turning of LEDs\r\n>");
		} else if (strstr(telnet_cmd,"down")) {
			ht1632c_set_brightness(0);
			memset(telnet_cmd, 0, 50);
			at_cipsend("Dimming down LEDs\r\n>");
		} else if (strstr(telnet_cmd,"up")) {
			ht1632c_set_brightness(15);
			memset(telnet_cmd, 0, 50);
			at_cipsend("Dimming up LEDs\r\n>");
		} else {
			memset(telnet_cmd, 0, 50);
			at_cipsend("Command not found \r\n>");
		}
	} else if (strstr(telnet_cmd,"get")) {
		if (strstr(telnet_cmd,"password")) {
			memset(telnet_cmd, 0, 50);
			at_cipsend(PASS);
			at_cipsend("\r\n>");
		} else if (strstr(telnet_cmd,"ssid")) {
			memset(telnet_cmd, 0, 50);
			at_cipsend(SSID);
			at_cipsend("\r\n>");
		} else {
			memset(telnet_cmd, 0, 50);
			at_cipsend("Command not found \r\n>");
		}
	} else if (strstr(telnet_cmd,"get")) {
		if (strstr(telnet_cmd,"password")) {
			memset(telnet_cmd, 0, 50);
			at_cipsend(PASS);
			at_cipsend("\r\n>");
		} else if (strstr(telnet_cmd,"ssid")) {
			memset(telnet_cmd, 0, 50);
			at_cipsend(SSID);
			at_cipsend("\r\n>");
		} else {
			memset(telnet_cmd, 0, 50);
			at_cipsend("Command not found \r\n>");
		}
	} else if (strstr(telnet_cmd,"set")) {
		if (strstr(telnet_cmd,"time")) {
			time_t time;
			uint16_t timeout = 1000;
			uint16_t cnt = 0;
			char *token;
			memset(telnet_cmd, 0, 50);
			at_cipsend("Date format - [hh:mm:ss:yyyy]\r\n>");
			
			while(!got_reply && (++cnt < timeout)) {
				_delay_ms(60);
			}
			
			if (strlen(telnet_cmd) == 24) {
				
				token = strtok(telnet_cmd,":");
				if (token == NULL) {
					at_cipsend("Wrong time format!\r\n");
					return ESP8266_ERROR;
				}
			
				token = strtok(NULL,":");
				if (token == NULL) {
					at_cipsend("Wrong time format...failed at hours!\r\n");
					return ESP8266_ERROR;
				}
				
				time.hours = atoi(token);
				if (time.hours > 23) {
					at_cipsend("Wrong time format...failed at hours!\r\n");
					return ESP8266_ERROR;
				}
		
				token = strtok(NULL,":");
				if (token == NULL) {
					at_cipsend("Wrong time format...failed at minutes!\r\n");
					return ESP8266_ERROR;
				}
				
				time.minutes = atoi(token);
				if (time.minutes > 59) {
					at_cipsend("Wrong time format...failed at minutes!\r\n");
					return ESP8266_ERROR;
				}
				
				token = strtok(NULL,":");
				if (token == NULL) {
					at_cipsend("Wrong time format...failed at seconds!\r\n");
					return ESP8266_ERROR;
				}
				
				time.seconds = atoi(token);
				if (time.seconds > 59) {
					at_cipsend("Wrong time format...failed at seconds!\r\n");
					return ESP8266_ERROR;
				}
				
				token = strtok(NULL,":");
				if (token == NULL) {
					at_cipsend("Wrong time format...failed at years!\r\n");
					return ESP8266_ERROR;
				}
				time.year = atoi(token);
				rtc_set_time(time.seconds,time.minutes,time.hours,time.days,time.year);	
				at_cipsend("Time was successfully set!\r\n");
			} else {
				at_cipsend("Wrong time format or timeout!\r\n");
			}
			
		} else if (strstr(telnet_cmd,"ssid")) {
			memset(telnet_cmd, 0, 50);
			at_cipsend(SSID);
			at_cipsend("\r\n");
		} else {
			memset(telnet_cmd, 0, 50);
			at_cipsend("Not a valid command\r\n");
		}
	} else if (strstr(telnet_cmd,"wifi")) {
		if (strstr(telnet_cmd,"off")) {
			memset(telnet_cmd, 0, 50);
			at_cipsend("Turning off wifi. Use BTN1 to turn it on again.\r\n");
			esp8266_off();
		} else {
			memset(telnet_cmd, 0, 50);
			at_cipsend("Command not found \r\n");
		}
	} else if (strstr(telnet_cmd,"man")) {
		memset(telnet_cmd, 0, 50);
		at_cipsend("-----------------------------------\r\n");
		at_cipsend("led on - Turns the entire LED array on.\r\n");
		at_cipsend("led off - Turns the entire LED array off.\r\n");
		at_cipsend("-----------------------------------\r\n");
		at_cipsend("set time - Sets the time and date of the clock. Format [hh:mm:ss:yyyy]\r\n");
		at_cipsend("set ssid - Sets the SSID for the WiFi connection \r\n");
		at_cipsend("set password - Sets the PASSWORD for the WiFi connection. \r\n");
		at_cipsend("-----------------------------------\r\n");
		at_cipsend("get time - Show the time and date of the clock. \r\n");
		at_cipsend("get ssid - Show the SSID for the WiFi connection \r\n");
		at_cipsend("get password - Show the PASSWORD for the WiFi connection. \r\n");
		at_cipsend("-----------------------------------\r\n");
	}
}

esp8266_status_t esp8266_run_simple_webserver(char *str) 
{
	if(rx_ptr > 190) // check if the ESP8266 is sending data
	{
		_delay_ms(1000);
		at_cipsend(str);
		_delay_ms(10000);
		esp8266_send_cmd("AT+CIPCLOSE=0\r", 100);
	}
}

ISR(USARTD0_RXC_vect) {
	
	char rx_temp = USARTD0.DATA;
	
	if (rx_temp == '\n') {
		if (strstr(rx_buffer,"OK")) {
			status = ESP8266_SUCCESS;
		} else if (strstr(rx_buffer,"ERROR")) {
			status = ESP8266_ERROR;
		} else if (strstr(rx_buffer,"CONNECT")) {
			status = ESP8266_CONNECT;
		} else if (strstr(rx_buffer,"CLOSED")) {
			status = ESP8266_CLOSED;
		} else if (strstr(rx_buffer,"192")) {
			strncpy(ip_address,strchr(rx_buffer,'\"'),19);
		//This might be error prone...
		} else if (strstr(rx_buffer,"+IPD")) {
			got_reply = true;
			strncpy(telnet_cmd,rx_buffer,50);
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
	}
}