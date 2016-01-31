/*
 * uart.h
 *
 * Created: 10/25/2014 11:25:50 PM
 *  Author: Administrator
 */ 

#ifndef _UART_H_
#define _UART_H_

#include "../../global.h"

#define UART_PORT	PORTD
#define UART_TX		PIN3_bm
#define UART_RX		PIN2_bm

void uart_setup(void);
void uart_put_char(char c);
char uart_get_char(void);
void uart_write_str(char *str);
void uart_write_hex(unsigned char n);

#endif