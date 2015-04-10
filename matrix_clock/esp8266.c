/*
 * esp8266.c
 *
 * Created: 1/18/2015 1:42:28 PM
 *  Author: Administrator
 */ 
#include "esp8266.h"
#include "ht1632c.h"

#include <avr/interrupt.h>

char cmd[BUFFER];
char buffer[BUFFER];
volatile uint16_t rx_ptr = 0;
char *str;
volatile uint8_t download = 0;

static inline void flush_rx_buffer(void) {
	rx_ptr = 0;
	memset(rx_buffer, 0, RX_BUFFER); //Empty buffer
}

static inline void flush_cmd_buffer(void) {
	memset(cmd, 0, BUFFER);
}

static inline void esp8266_send_cmd(char *str, uint16_t timeout_ms) {
	flush_rx_buffer();
	puts(str);
	//printf("%s \r\n",str);
	_delay_ms(timeout_ms);
}

esp8266_status_t esp8266_init_webserver(void) {
	esp8266_send_cmd("AT+RST", 2000);
	esp8266_send_cmd("AT+CWMODE=2", 2000);
	esp8266_send_cmd("AT+CIFSR", 2000);
	esp8266_send_cmd("AT+CIPMUX=1", 2000);
	esp8266_send_cmd("AT+CIPSERVER=1,80", 2000);
}

esp8266_status_t esp8266_setup(void) {
    
	//Reset module
	esp8266_send_cmd("AT+RST",2000);
	
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
	
	//Join Access-Point
	flush_cmd_buffer();
	strcat(cmd,"AT+CWJAP=\"");
	strcat(cmd,SSID);
	strcat(cmd,"\",\"");
	strcat(cmd,PASS);
	strcat(cmd,"\"");
	esp8266_send_cmd(cmd,5000);
	esp8266_send_cmd("AT+CWJAP=?",5000);
	
	//Set up TCP connection
	flush_cmd_buffer();
	strcat(cmd,"AT+CIPSTART=\"TCP\",\"");
	strcat(cmd,DST_IP);
	strcat(cmd,"\",80");
	esp8266_send_cmd(cmd,2000);
	
	//Send number of bytes
	//flush_cmd_buffer();
	//char *str;
	//sprintf(str, "%d", strlen(ADDRESS)+12);
	//strcat("AT+CIPSEND=",str);
	//strcat(cmd,str);
	printf("AT+CIPSEND=110\r\n");
	
	//Send data
	flush_cmd_buffer();
	strcat(cmd,"GET ");
	strcat(cmd,ADDRESS);
	strcat(cmd," HTTP/1.0");
	esp8266_send_cmd(cmd,2000);
	
	uint16_t timeout = 40;
	uint16_t counter = 0;
	while (strstr(rx_buffer,"SEND OK") == NULL)
	{
		esp8266_send_cmd("",250);
		
		if (counter++ > timeout) {
			return SUCCESS;
		}
	}
	
}

static inline void esp8266_reset(void) {
	PORTD.OUTSET = RST; //| PIN5_bm;_
	_delay_ms(100);
	PORTD.OUTCLR = RST;
	_delay_ms(100);
}

void esp8266_on(void) {
	PORTD.DIRSET = CH_EN;
	PORTD.OUTSET = CH_EN;
	_delay_ms(3000);
	//esp8266_reset();
}

void esp8266_off(void) {
	PORTD.OUTCLR |= PIN4_bm | PIN5_bm;
}

void esp8266_connect(void)
{
  flush_rx_buffer();
  flush_cmd_buffer();
  
  strcat(cmd,"AT+CIPSTART=\"TCP\",\"");
  strcat(cmd,DST_IP);
  strcat(cmd,"\",80");
  puts(cmd);
  
  _delay_ms(5000);
  
  flush_rx_buffer();
  flush_cmd_buffer();
  
  strcat(cmd,"GET ");
  strcat(cmd,ADDRESS);
  strcat(cmd," HTTP/1.0\r\nHost: ");
  strcat(cmd,DST_IP);
  strcat(cmd,"\r\n\r\n\r\n");
  
  puts("AT+CIPSEND=");
  char str[15];
  sprintf(str, "%d", strlen(cmd));
  puts(str); //Number of bytes
  puts(cmd);
  
  _delay_ms(5000);

  memset(cmd, 0, BUFFER); //Empty buffer
  //uart_write_str(rx_buffer);

}

esp8266_status_t connectWiFi(void) {
	
  return SUCCESS;
}

ISR(USARTD0_RXC_vect) {
	rx_buffer[rx_ptr++] = USARTD0.DATA;
	download = 1;
}