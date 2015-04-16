/*
 * esp8266.c
 *
 * Created: 1/18/2015 1:42:28 PM
 *  Author: Administrator
 */ 
#include "esp8266.h"
#include "ht1632c.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

char cmd[BUFFER];

volatile static uint16_t rx_ptr = 0;
static char rx_buffer[RX_BUFFER];

static inline void flush_rx_buffer(void) {
	rx_ptr = 0;
	memset(rx_buffer, 0, RX_BUFFER);
}

static inline void flush_cmd_buffer(void) {
	memset(cmd, 0, BUFFER);
}

static inline void esp8266_send_cmd(char *str, uint16_t timeout_ms) {
	flush_rx_buffer();
	//puts(str);
	printf("%s\r\n",str);
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
	PORTD.OUTSET = ESP_RST; //| PIN5_bm;_
	_delay_ms(100);
	PORTD.OUTCLR = ESP_RST;
	_delay_ms(100);
}

esp8266_status_t esp8266_setup(void) {
    
	//Reset module
	esp8266_send_cmd("AT+RST\r",5000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Set Data Mode
	esp8266_send_cmd("AT+CIPMODE=0",1000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Single connection mode
	esp8266_send_cmd("AT+CIPMUX=0",1000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Select mode
	esp8266_send_cmd("AT+CWMODE=1",1000);
	
	return SUCCESS;
}

esp8266_status_t esp8266_join_ap(char *ssid, char *pass) {
	flush_cmd_buffer();
	strcat(cmd,"AT+CWJAP=\"");
	strcat(cmd,ssid);
	strcat(cmd,"\",\"");
	strcat(cmd,pass);
	strcat(cmd,"\"");
	esp8266_send_cmd(cmd,5000);
	esp8266_send_cmd("AT+CWJAP=?",5000);
	
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}

	return SUCCESS;
}

esp8266_status_t esp8266_connect(char *host, char *addr) {
	//Set up TCP connection to host
	flush_cmd_buffer();
	strcat(cmd,"AT+CIPSTART=\"TCP\",\"");
	strcat(cmd,host);
	strcat(cmd,"\",80");
	esp8266_send_cmd(cmd,2000);
	
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Count number of bytes to send
	flush_cmd_buffer();
	char *number_of_bytes;
	sprintf(number_of_bytes, "%d", strlen(addr)+15);
	strcat(cmd, "AT+CIPSEND=");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here
	esp8266_send_cmd(cmd,1000);
	
	//Send data
	flush_cmd_buffer();
	strcat(cmd,"GET ");
	strcat(cmd,addr);
	strcat(cmd," HTTP/1.0");
	esp8266_send_cmd(cmd,1000);
	
	uint16_t timeout = 40;
	uint16_t cnt = 0;
	
	while (strstr(rx_buffer,"SEND") == NULL)
	{
		esp8266_send_cmd("",250);
		
		if (cnt++ > timeout) {
			return TIMEOUT;
		}
	}
	_delay_ms(5000);
	/*
	cnt = 0;
	while (strstr(rx_buffer,"Unlink") == NULL)
	{
		_delay_ms(250);
		if (cnt++ > timeout) {
			return TIMEOUT;
		}
	}*/
	
	return SUCCESS;
}

static inline void remove_substring(char *src, char *sub) {
	char *p;
	if ((p=strstr(src,sub)) != NULL) {
		memmove(p,p+strlen(sub), strlen(p+strlen(sub))+1);
		// alternative
		// strcpy(p,p+strlen(sub));
	}
}

void esp8266_get_rx_buffer(char *str) {
	
	strncpy(str,rx_buffer,strlen(rx_buffer));
}

void esp8266_extract_json(char *str) {
	
	char *tmp;
	char *tmp2;
	
	tmp = strchr(rx_buffer,'{');
	tmp2 = strrchr(tmp,'}');
	*tmp2++;
	
	remove_substring(tmp,tmp2);
	
	strncpy(str,tmp,strlen(tmp));
}

esp8266_status_t esp8266_setup_webserver(void) {
	esp8266_send_cmd("AT+RST\r", 5000);
	esp8266_send_cmd("AT+CIPMUX=1", 2000);
	esp8266_send_cmd("AT+CWMODE=2", 2000);
	esp8266_send_cmd("AT+CIPSERVER=1,80", 2000);
	esp8266_send_cmd("AT+CIFSR=?", 2000);
	
	return SUCCESS;
}

ISR(USARTD0_RXC_vect) {
	rx_buffer[rx_ptr++] = USARTD0.DATA;
}