/*
 * uart.h
 *
 * Created: 10/25/2014 11:25:50 PM
 *  Author: Administrator
 */ 

#define TIMEOUT 3000

void uart_setup(void);
void uart_put_char(char c);
char uart_get_char(void);
void uart_write_str(char *str);
void uwrite_hex(unsigned char n);