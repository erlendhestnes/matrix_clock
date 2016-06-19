/*
 * uart.c
 *
 * Created: 10/25/2014 11:25:42 PM
 *  Author: Administrator
 */ 

#include "uart.h"

void uart_setup(void) 
{
	//Disable power reduction for USARTD0 
	PR.PRPD &= ~0x10;
		
	PORTD.DIRCLR = UART_RX;
	PORTD.DIRSET = UART_TX;
	
	PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc;
	
	USARTD0.CTRLA	  = USART_RXCINTLVL_MED_gc;
	USARTD0.BAUDCTRLA = 2094;
	USARTD0.BAUDCTRLB = (-7 << 4) | (2094 >> 8);
	USARTD0.CTRLC	  = USART_CHSIZE_8BIT_gc;
	USARTD0.CTRLB	  = USART_RXEN_bm | USART_TXEN_bm;
}

void uart_setup_debug(void)
{
	//Disable power reduction for USARTD0
	PR.PRPD &= ~0x20;
	
	PORTD.DIRCLR = PIN6_bm;
	PORTD.DIRSET = PIN7_bm;
	
	PORTD.PIN6CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	USARTD1.CTRLA	  = USART_RXCINTLVL_OFF_gc;
	USARTD1.BAUDCTRLA = 2094;
	USARTD1.BAUDCTRLB = (-7 << 4) | (2094 >> 8);
	USARTD1.CTRLC	  = USART_CHSIZE_8BIT_gc;
	USARTD1.CTRLB	  = USART_RXEN_bm | USART_TXEN_bm;
}

void uart_disable(void)
{	
	USARTD0.CTRLB = 0;
	
	PORTD.DIRCLR   = UART_TX | UART_RX;
	PORTD.PIN2CTRL = PORT_OPC_PULLDOWN_gc;
	PORTD.PIN3CTRL = PORT_OPC_PULLDOWN_gc;
	
	//Enable power reduction for USARTD0 
	PR.PRPD |= 0x10;
}

void uart_put_char(char c) 
{
	while (!(USARTD0.STATUS & USART_DREIF_bm));
	USARTD0.DATA = c;
}

char uart_get_char(void) 
{
	while (!(USARTD0.STATUS & USART_RXCIF_bm));
	return USARTD0.DATA;
}

void uart_put_char_debug(char c)
{
	while (!(USARTD1.STATUS & USART_DREIF_bm));
	USARTD1.DATA = c;
}

char uart_get_char_debug(void)
{
	while (!(USARTD1.STATUS & USART_RXCIF_bm));
	return USARTD1.DATA;
}

void uart_write_str(char *str) 
{
	while (*str) {
		uart_put_char(*str++);
	}
	uart_put_char('\r');
	uart_put_char('\n');
}

void uart_write_hex(unsigned char c) 
{
	if(((c>>4) & 15) < 10)
	uart_put_char('0' + ((c>>4)&15));
	else
	uart_put_char('A' + ((c>>4)&15) - 10);
	c <<= 4;
	if(((c>>4) & 15) < 10)
	uart_put_char('0' + ((c>>4)&15));
	else
	uart_put_char('A' + ((c>>4)&15) - 10);
	
	uart_put_char('\r');
	uart_put_char('\n');
}