/*
 * uart.c
 *
 * Created: 10/25/2014 11:25:42 PM
 *  Author: Administrator
 */ 

#define F_CPU 32000000UL

#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>

char rxbuf[50];

void uart_setup(void) {
	
	PORTD.DIRSET = PIN3_bm; //TX
	PORTD.DIRCLR = PIN2_bm; //RX
	
	USARTD0.CTRLA = USART_RXCINTLVL_MED_gc;
	USARTD0.BAUDCTRLA = 207;//0x1111;
	//USARTE0.BAUDCTRLB = (33 << 4);
	USARTD0.CTRLC = USART_CHSIZE_8BIT_gc;

	USARTD0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

void uart_put_char(char c) {
	while (!(USARTD0.STATUS & USART_DREIF_bm)){}
	USARTD0.DATA = c;
}

char uart_get_char(void) {
	uint16_t timer = 0;
	while (!(USARTD0.STATUS & USART_RXCIF_bm)){
		 _delay_ms(1);
		 if (timer++ == TIMEOUT) {
			return 0x03;
		 }
	}
	return USARTD0.DATA;
}

void uart_write_str(char *str) {
	while (*str) {
		uart_put_char(*str++);
	}
}

void uwrite_hex(unsigned char n) {
	if(((n>>4) & 15) < 10)
	uart_put_char('0' + ((n>>4)&15));
	else
	uart_put_char('A' + ((n>>4)&15) - 10);
	n <<= 4;
	if(((n>>4) & 15) < 10)
	uart_put_char('0' + ((n>>4)&15));
	else
	uart_put_char('A' + ((n>>4)&15) - 10);
}