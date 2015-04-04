/*
 * ht1632c.c
 *
 * Created: 10/9/2014 5:50:34 PM
 *  Author: Administrator
 */ 
#define F_CPU 32000000UL

#include "ht1632c.h"
#include "4x6_font.c"
#include <util/delay.h>
#include <avr/sfr_defs.h>

//PC4 - SS -> EXT1_15 -> CS
//PC5 - MOSI -> EXT1_16 -> DATA
//PC6 - MISO -> EXT1_17 -> READ
//PC7 - SCK -> EXT1_18 -> WRITE

#define swap(a, b) { uint16_t t = a; a = b; b = t; }

#define HT1632_PORT PORTE

#define _cs PIN0_bm
#define _rd PIN1_bm
#define _wr PIN2_bm
#define _data PIN3_bm

#define WIDTH 24
#define HEIGHT 16

uint8_t ledmatrix[48]; // 16 * 24 / 8

uint8_t cursor_x;
uint8_t cursor_y;
uint8_t textsize = 1;

void ht1632c_begin(uint8_t type) {
	HT1632_PORT.DIRSET = _cs;
	HT1632_PORT.OUTSET = _cs;
	
	HT1632_PORT.DIRSET = _wr;
	HT1632_PORT.OUTSET = _wr;
	
	HT1632_PORT.DIRSET = _data;
	
	/* READ
	HT1632_PORT.DIRSET = _rd;
	HT1632_PORT.OUTSET = _rd;
	*/

	ht1632c_sendcommand(HT1632_SYS_EN);
	ht1632c_sendcommand(HT1632_LED_ON);
	ht1632c_sendcommand(HT1632_BLINK_OFF);
	ht1632c_sendcommand(HT1632_MASTER_MODE);
	ht1632c_sendcommand(HT1632_INT_RC);
	ht1632c_sendcommand(type);
	ht1632c_sendcommand(HT1632_PWM_CONTROL | 0x1);
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
	
	ht1632c_sendcommand(HT1632_LED_OFF);
	ht1632c_sendcommand(HT1632_SYS_DIS);
}

void ht1632c_setBrightness(uint8_t pwm) {
	if (pwm > 15) {
		pwm = 15;	
	}
	ht1632c_sendcommand(HT1632_PWM_CONTROL | pwm);
}

void ht1632c_blink(uint8_t blinky) {
	if (blinky) {
		ht1632c_sendcommand(HT1632_BLINK_ON);	
	} else {
		ht1632c_sendcommand(HT1632_BLINK_OFF);	
	}
}

void ht1632c_drawPixel(uint8_t x, uint8_t y, uint8_t color) {
	if (y >= WIDTH) {
		return;
	}
	if (x >= HEIGHT) {
		return;
	}

	uint8_t m;
	// figure out which matrix controller it is
	m = x / 24;
	x %= 24;

	uint16_t i;

	if (x < 8) {
		i = 7;
		} else if (x < 16) {
		i = 128 + 7;
		} else {
		i = 256 + 7;
	}
	i -= (x % 8);

	if (y < 8) {
		y *= 2;
		} else {
		y = (y-8) * 2 + 1;
	}

	i += y * 8;

	if (color)
	ht1632c_setPixel(i);
	else
	ht1632c_clrPixel(i);
}



void ht1632c_drawBitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color) {
	for (uint8_t j=0; j<h; j++) {
		for (uint8_t i=0; i<w; i++ ) {
			if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) {
				ht1632c_drawPixel(x+i, y+j, color);
			}
		}
	}
}

// draw a character
void ht1632c_drawChar(uint8_t x, uint8_t y, char c,uint16_t color, uint8_t size) {
  for (uint8_t i =0; i<5; i++ ) {
	  uint8_t line = pgm_read_byte(font+(c*5)+i);
	  for (uint8_t j = 0; j<8; j++) {
		  if (line & 0x1) {
			  if (size == 1) // default size
			  ht1632c_drawPixel(y+j, x+i, color);
			  else {  // big size
				  ht1632c_fillRect(y+j*size, x+i*size, size, size, color);
			  }
		  }
		  line >>= 1;
	  }
  }
}
/*
void ht1632c_write_char(uint8_t base, uint8_t character) {
	
	uint8_t i = 0;
	base = 16*base;
	
	if (base > 17) {
		base += 2;
	}
	
	for (i = 0; i < 4; i++) {
		uint8_t font_data = pgm_read_byte(&font[character-32][3-i]);
		ht1632c_writeRAM(base+i*4,font_data >> 4);
		ht1632c_writeRAM(base+1+i*4,font_data); 
	}
}
*/

// bresenham's algorithm - thx wikpedia
void ht1632c_drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color) {
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
			ht1632c_drawPixel(y0, x0, color);
			} else {
			ht1632c_drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

// fill a rectangle
void ht1632c_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color) {
	for (uint8_t i=x; i<x+w; i++) {
		for (uint8_t j=y; j<y+h; j++) {
			ht1632c_drawPixel(i, j, color);
		}
	}
}

// draw a circle outline
void ht1632c_drawCircle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ht1632c_drawPixel(x0, y0+r, color);
	ht1632c_drawPixel(x0, y0-r, color);
	ht1632c_drawPixel(x0+r, y0, color);
	ht1632c_drawPixel(x0-r, y0, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		ht1632c_drawPixel(x0 + x, y0 + y, color);
		ht1632c_drawPixel(x0 - x, y0 + y, color);
		ht1632c_drawPixel(x0 + x, y0 - y, color);
		ht1632c_drawPixel(x0 - x, y0 - y, color);
		
		ht1632c_drawPixel(x0 + y, y0 + x, color);
		ht1632c_drawPixel(x0 - y, y0 + x, color);
		ht1632c_drawPixel(x0 + y, y0 - x, color);
		ht1632c_drawPixel(x0 - y, y0 - x, color);
		
	}
}


// fill a circle
void ht1632c_fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ht1632c_drawLine(x0, y0-r, x0, y0+r+1, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		ht1632c_drawLine(x0+x, y0-y, x0+x, y0+y+1, color);
		ht1632c_drawLine(x0-x, y0-y, x0-x, y0+y+1, color);
		ht1632c_drawLine(x0+y, y0-x, x0+y, y0+x+1, color);
		ht1632c_drawLine(x0-y, y0-x, x0-y, y0+x+1, color);
	}
}

void ht1632c_setPixel(uint16_t i) {
	ledmatrix[i/8] |= _BV(i%8);
}

void ht1632c_clrPixel(uint16_t i) {
	ledmatrix[i/8] &= ~_BV(i%8);
}

void ht1632c_writeScreen() {

	HT1632_PORT.OUTCLR = _cs;

	ht1632c_writedata(HT1632_WRITE, 3);
	// send with address 0
	ht1632c_writedata(0, 7);

	for (uint16_t i=0; i<(WIDTH*HEIGHT/8); i+=2) {
		uint16_t d = ledmatrix[i];
		d <<= 8;
		d |= ledmatrix[i+1];

		ht1632c_writedata(d, 16);
	}
	HT1632_PORT.OUTSET = _cs;
}


void ht1632c_clearScreen() {
	for (uint8_t i=0; i<(WIDTH*HEIGHT/8); i++) {
		ledmatrix[i] = 0;
	}
	ht1632c_writeScreen();
}

void ht1632c_writedata(uint16_t d, uint8_t bits) {
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




void ht1632c_writeRAM(uint8_t addr, uint8_t data) {
	uint16_t d = HT1632_WRITE;
	d <<= 7;
	d |= addr & 0x7F;
	d <<= 4;
	d |= data & 0xF;
		
	HT1632_PORT.OUTCLR = _cs;
	ht1632c_writedata(d, 14);
	HT1632_PORT.OUTSET = _cs;
}


void ht1632c_sendcommand(uint8_t cmd) {
	uint16_t data = 0;
	data = HT1632_COMMAND;
	data <<= 8;
	data |= cmd;
	data <<= 1;
		
	HT1632_PORT.OUTCLR = _cs;
	ht1632c_writedata(data, 12);
	HT1632_PORT.OUTSET = _cs;
}


void ht1632c_fillScreen() {
	for (uint8_t i=0; i<(WIDTH*HEIGHT/8); i++) {
		ledmatrix[i] = 0xFF;
	}
	ht1632c_writeScreen();
}

void traverse_screen(void) {
	static uint8_t x = 0;
	static uint8_t y = 0;
	
	ht1632c_drawPixel(x,y,1);
	ht1632c_writeScreen();
	x++;
	y += x / 16;
	x %= 16;
	y %= 16;
}

void traverse_screen_clear(void) {
	static uint8_t x = 0;
	static uint8_t y = 0;
	
	ht1632c_drawPixel(x,y,0);
	ht1632c_writeScreen();
	x++;
	y += x / 16;
	x %= 16;
	y %= 16;
}

void pwm_fade(void) {
	static uint8_t pwm = 0;
	static sw = 1;
	ht1632c_setBrightness(pwm);
	if (pwm < 15 && sw) {
		pwm++;
	} else if(pwm > 0) {
		pwm--;
		sw = 0;
	} else if (pwm == 0)
	{
		sw = 1;
	}
}

void fade_up(void) {
	uint8_t pwm = 0;
	while (pwm < 15)
	{
		ht1632c_setBrightness(pwm);
		_delay_ms(10);
		pwm++;
	}
}

void fade_down(void) {
	uint8_t pwm = 15;
	while (pwm > 0)
	{
		ht1632c_setBrightness(pwm);
		_delay_ms(10);
		pwm--;
	}
}

void ram_test(void) {
	ht1632c_clearScreen();
	ht1632c_writeRAM(0x22,0xff);
	ht1632c_writeRAM(0x23,0xff);
	ht1632c_writeRAM(0x26,0xff);
	ht1632c_writeRAM(0x27,0xff);
}

void ht1632c_setCursor(uint8_t x, uint8_t y) {
	cursor_x = x;
	cursor_y = y;
}

void ht1632c_write(uint8_t c) {
	if (c == '\n') {
		cursor_y += textsize;
		cursor_x = 0;
		} else if (c == '\r') {
		// skip em
		} else {
		ht1632c_drawChar(cursor_x, cursor_y, c, 1, textsize);
		cursor_x += textsize*6;
	}
	return 1;
}

void ht1632c_print(uint8_t *str) {
	while(*str) {
		ht1632c_write(*str++);
	}
	ht1632c_writeScreen();
}

void ht1632_random(void) {
	static volatile uint8_t i = 0;
	ht1632c_writeRAM(i++,rand());
	i %= 64;
	//pwm_fade();
}
