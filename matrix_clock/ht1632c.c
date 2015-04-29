/*
 * ht1632c.c
 *
 * Created: 10/9/2014 5:50:34 PM
 *  Author: Administrator
 */ 

#include "global.h"
#include "ht1632c.h"
#include "4x6_font.c"
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

#define swap(a, b) { uint16_t t = a; a = b; b = t; }

#define HT1632_PORT PORTE

#define _cs PIN0_bm
#define _rd PIN1_bm
#define _wr PIN2_bm
#define _data PIN3_bm

#define WIDTH 16
#define HEIGHT 16

static uint8_t ledmatrix[48]; // 16 * 24 / 8

static int16_t cursor_x;
static int16_t cursor_y;

uint8_t textsize = 1;

/*--------------------------------SETUP-----------------------------------*/

void ht1632c_setup(uint8_t type) {
	
	HT1632_PORT.DIRSET = _cs;
	HT1632_PORT.OUTSET = _cs;
	
	HT1632_PORT.DIRSET = _wr;
	HT1632_PORT.OUTSET = _wr;
	
	HT1632_PORT.DIRSET = _data;
	
	/* READ - Not used
	HT1632_PORT.DIRSET = _rd;
	HT1632_PORT.OUTSET = _rd;
	*/

	ht1632c_send_command(HT1632_SYS_EN);
	ht1632c_send_command(HT1632_LED_ON);
	ht1632c_send_command(HT1632_BLINK_OFF);
	ht1632c_send_command(HT1632_MASTER_MODE);
	ht1632c_send_command(HT1632_INT_RC);
	ht1632c_send_command(type);
	ht1632c_send_command(HT1632_PWM_CONTROL | 0x1);
}

void ht1632c_powerdown(void) {
	
	HT1632_PORT.DIRCLR = _cs;
	HT1632_PORT.DIRCLR = _wr;
	HT1632_PORT.DIRCLR = _rd;
	HT1632_PORT.DIRCLR = _data;
	
	HT1632_PORT.PIN4CTRL = PORT_OPC_PULLUP_gc;
	HT1632_PORT.PIN5CTRL = PORT_OPC_PULLUP_gc;
	HT1632_PORT.PIN6CTRL = PORT_OPC_PULLUP_gc;
	HT1632_PORT.PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	ht1632c_send_command(HT1632_LED_OFF);
	ht1632c_send_command(HT1632_SYS_DIS);
}

void ht1632c_set_brightness(uint8_t pwm) {
	
	if (pwm > 15) {
		pwm = 15;	
	}
	ht1632c_send_command(HT1632_PWM_CONTROL | pwm);
}

void ht1632c_blink(uint8_t blinky) {
	
	if (blinky) {
		ht1632c_send_command(HT1632_BLINK_ON);	
	} else {
		ht1632c_send_command(HT1632_BLINK_OFF);	
	}
}

/*-------------------------Geometric functions----------------------------*/

void ht1632c_set_pixel(uint16_t i) {
	ledmatrix[i/8] |= _BV(i%8);
}

void ht1632c_clr_pixel(uint16_t i) {
	ledmatrix[i/8] &= ~_BV(i%8);
}

void ht1632c_draw_pixel(int x, int y, uint8_t color) {
	
	if (x >= WIDTH) {
		return;
	}
	if (y >= HEIGHT) {
		return;
	}
	y = 15 - y;

	y %= 24;

	int i;

	if (y < 8) {
		i = 7;
	} else if (y < 16) {
		i = 128 + 7;
	} else {
		i = 256 + 7;
	}
	i -= (y % 8);

	if (x < 8) {
		x *= 2;
	} else {
		x = (x-8) * 2 + 1;
	}

	i += x * 8;

	if (color)
	ht1632c_set_pixel(i);
	else
	ht1632c_clr_pixel(i);
}

void ht1632c_draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color) {

	uint16_t steep = abs(y1 - y0) > abs(x1 - x0);
	
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	uint16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
		} else {
	ystep = -1;}

	for (; x0<=x1; x0++) {
		if (steep) {
			ht1632c_draw_pixel(y0, x0, color);
			} else {
			ht1632c_draw_pixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void ht1632c_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color) {
	
	for (uint8_t i=x; i<x+w; i++) {
		for (uint8_t j=y; j<y+h; j++) {
			ht1632c_draw_pixel(i, j, color);
		}
	}
}

void ht1632c_draw_circle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color) {
	
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ht1632c_draw_pixel(x0, y0+r, color);
	ht1632c_draw_pixel(x0, y0-r, color);
	ht1632c_draw_pixel(x0+r, y0, color);
	ht1632c_draw_pixel(x0-r, y0, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		ht1632c_draw_pixel(x0 + x, y0 + y, color);
		ht1632c_draw_pixel(x0 - x, y0 + y, color);
		ht1632c_draw_pixel(x0 + x, y0 - y, color);
		ht1632c_draw_pixel(x0 - x, y0 - y, color);
		
		ht1632c_draw_pixel(x0 + y, y0 + x, color);
		ht1632c_draw_pixel(x0 - y, y0 + x, color);
		ht1632c_draw_pixel(x0 + y, y0 - x, color);
		ht1632c_draw_pixel(x0 - y, y0 - x, color);
		
	}
}

void ht1632c_fill_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {

	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ht1632c_draw_line(x0, y0-r, x0, y0+r+1, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		ht1632c_draw_line(x0+x, y0-y, x0+x, y0+y+1, color);
		ht1632c_draw_line(x0-x, y0-y, x0-x, y0+y+1, color);
		ht1632c_draw_line(x0+y, y0-x, x0+y, y0+x+1, color);
		ht1632c_draw_line(x0-y, y0-x, x0-y, y0+x+1, color);
	}
}

void ht1632c_draw_bitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color) {
	
	for (uint8_t j=0; j<h; j++) {
		for (uint8_t i=0; i<w; i++ ) {
			if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) {
				ht1632c_draw_pixel(x+i, y+j, color);
			}
		}
	}
}

/*-------------------------REFRESH FUNCTIONS-----------------------------------*/

void ht1632c_refresh_screen() {

	HT1632_PORT.OUTCLR = _cs;

	ht1632c_write_data(HT1632_WRITE, 3);
	// send with address 0
	ht1632c_write_data(0, 7);

	for (uint16_t i=0; i<(WIDTH*HEIGHT/8); i+=2) {
		uint16_t d = ledmatrix[i];
		d <<= 8;
		d |= ledmatrix[i+1];

		ht1632c_write_data(d, 16);
	}
	HT1632_PORT.OUTSET = _cs;
}

void ht1632c_fill_screen() {

	for (uint8_t i=0; i<(WIDTH*HEIGHT/8); i++) {
		ledmatrix[i] = 0xFF;
	}
	ht1632c_refresh_screen();
}

void ht1632c_clear_screen() {

	for (uint8_t i=0; i<(WIDTH*HEIGHT/8); i++) {
		ledmatrix[i] = 0;
	}
	ht1632c_refresh_screen();
}

static inline void ht1632c_clear_buffer() {

	for (uint8_t i=0; i<(WIDTH*HEIGHT/8); i++) {
		ledmatrix[i] = 0;
	}
}

void ht1632c_write_data(uint16_t d, uint8_t bits) {
	HT1632_PORT.DIRSET = _data;
	for (uint8_t i=bits; i > 0; i--) {
		HT1632_PORT.OUTCLR = _wr;
		if (d & _BV(i-1)) {
			HT1632_PORT.OUTSET = _data;
			} else {
			HT1632_PORT.OUTCLR = _data;
		}
		HT1632_PORT.OUTSET = _wr;
	}
	HT1632_PORT.OUTCLR = _data;
}

void ht1632c_write_ram(uint8_t addr, uint8_t data) {
	uint16_t d = HT1632_WRITE;
	d <<= 7;
	d |= addr & 0x7F;
	d <<= 4;
	d |= data & 0xF;
		
	HT1632_PORT.OUTCLR = _cs;
	ht1632c_write_data(d, 14);
	HT1632_PORT.OUTSET = _cs;
}

void ht1632c_send_command(uint8_t cmd) {
	uint16_t data = 0;
	data = HT1632_COMMAND;
	data <<= 8;
	data |= cmd;
	data <<= 1;
		
	HT1632_PORT.OUTCLR = _cs;
	ht1632c_write_data(data, 12);
	HT1632_PORT.OUTSET = _cs;
}

/*------------------------------STRING FUNCTIONS------------------------------*/

void ht1632c_set_cursor(int16_t x, int16_t y) {
	cursor_x = x;
	cursor_y = y;
}

void ht1632c_draw_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size) {
	
	uint8_t i;
	uint8_t j;
	
	for (i = 0; i < 5; i++) {
		uint8_t line = pgm_read_byte(font+(c*5)+i);
		for (j = 7; j > 0; j--) {
			if (line & 0x1) {
				if (size == 1) {
					if (x+i >= 0) {
						ht1632c_draw_pixel(x+i, y+j-1, color);
					}
				}
				else {  // big size
					ht1632c_fill_rect(x+i*size, y+j*size, size, size, color);
				}
			}
			line >>= 1;
		}
	}
}

void ht1632c_print(uint8_t *str) {
	
	ht1632c_clear_buffer();
	while(*str) {
		ht1632c_draw_char(cursor_x, cursor_y, *str++, 1, textsize);
		cursor_x += textsize*6;
	}
	ht1632c_refresh_screen();
}

void ht1632c_scroll_print(uint8_t *str, uint16_t len, uint16_t delay) {
	
	int16_t i;
	uint16_t length = strlen(str)*6;
	
	for (i = (WIDTH*2); i > -((int16_t)length); i--) {
		ht1632c_set_cursor(i,5);
		ht1632c_print(str);
		_delay_ms(150);
	}
}

/*---------------------------------DEBUG-------------------------------------*/

void ht1632_random(void) {

	static volatile uint8_t i = 0;
	ht1632c_write_ram(i++,rand());
	i %= 64;
	//pwm_fade();
}

void traverse_screen(void) {

	static uint8_t x = 0;
	static uint8_t y = 0;
	
	ht1632c_draw_pixel(x,y,1);
	ht1632c_refresh_screen();
	x++;
	y += x / 16;
	x %= 16;
	y %= 16;
}

void traverse_screen_clear(void) {

	static uint8_t x = 0;
	static uint8_t y = 0;
	
	ht1632c_draw_pixel(x,y,0);
	ht1632c_refresh_screen();
	x++;
	y += x / 16;
	x %= 16;
	y %= 16;
}

/*
void pwm_fade(void) {
	
	static uint8_t pwm = 0;
	static sw = 1;
	
	ht1632c_set_brightness(pwm);
	if (pwm < 15 && sw) {
		pwm++;
		} else if(pwm > 0) {
		pwm--;
		sw = 0;
	} else if (pwm == 0) {
		sw = 1;
	}
}
*/

static inline void fade_up(uint8_t pwm, uint8_t prev_pwm) {
	
	while (prev_pwm < pwm) {
		ht1632c_set_brightness(prev_pwm);
		_delay_ms(10);
		prev_pwm++;
	}
}

static inline void fade_down(uint8_t pwm, uint8_t prev_pwm) {
	
	while (prev_pwm > pwm) {
		ht1632c_set_brightness(prev_pwm);
		_delay_ms(10);
		prev_pwm--;
	}
}

void ht1632_fade(uint8_t pwm) {
	
	static uint8_t prev_pwm = 0;
	
	if (pwm > prev_pwm) {
		fade_up(pwm, prev_pwm);
	} else if (pwm < prev_pwm) {
		fade_down(pwm, prev_pwm);
	} else  {
		//do nothing
	}
	prev_pwm = pwm;
}