/*
 * ht1632c.c
 *
 * Created: 10/9/2014 5:50:34 PM
 *  Author: Administrator
 */ 

#include "ht1632c.h"
#include "../../4x6_font.c"

#define swap(a, b) { uint16_t t = a; a = b; b = t; }

#define HT1632_PORT PORTE

#define _cs PIN0_bm
#define _rd PIN1_bm
#define _wr PIN2_bm
#define _data PIN3_bm

#define WIDTH 16
#define HEIGHT 16

static uint8_t ledmatrix[32]; // 16 * 24 / 8

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
	
	HT1632_PORT.PIN0CTRL = PORT_OPC_PULLUP_gc;
	HT1632_PORT.PIN1CTRL = PORT_OPC_PULLUP_gc;
	HT1632_PORT.PIN2CTRL = PORT_OPC_PULLUP_gc;
	HT1632_PORT.PIN3CTRL = PORT_OPC_PULLUP_gc;
	
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

static inline void ht1632c_set_pixel(uint16_t i) {
	ledmatrix[i/8] |= _BV(i%8);
}

static inline void ht1632c_clr_pixel(uint16_t i) {
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

void ht1632c_draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color) {
	
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

void ht1632c_draw_filled_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {

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

	for (uint16_t i=0; i < 32; i+=2) {
		uint16_t d = ledmatrix[i];
		d <<= 8;
		d |= ledmatrix[i+1];

		ht1632c_write_data(d, 16);
	}
	HT1632_PORT.OUTSET = _cs;
}

void ht1632c_slide_in_from_left(void) {
	
	uint8_t temp[32];
	
	memset(temp,0,32);
	memcpy(temp,ledmatrix,32);
	memset(ledmatrix,0,32);
	
	int8_t i;
	
	for (i = 31; i >= 17; i -= 2)
	{
		ledmatrix[0] = temp[i-16];
		ledmatrix[16] = temp[i];
		ht1632c_shift_right();
	}
	
	for (i = 30; i >= 16; i -= 2)
	{
		ledmatrix[0] = temp[i-16];
		ledmatrix[16] = temp[i];
		ht1632c_shift_right();
	}
	
}

void ht1632c_slide_in_from_right(void) {
	
	uint8_t temp[32];
	
	memset(temp,0,32);
	memcpy(temp,ledmatrix,32);
	memset(ledmatrix,0,32);
	
	int8_t i;
	
	for (i = 0; i <= 14; i += 2)
	{
		ledmatrix[15] = temp[i];
		ledmatrix[31] = temp[i+16];
		ht1632c_shift_left();
	}
	
	for (i = 1; i <= 13; i += 2)
	{
		ledmatrix[15] = temp[i];
		ledmatrix[31] = temp[i+16];
		ht1632c_shift_left();
	}
	
}

void ht1632c_slide_out_to_right(void) {
	
	uint8_t i;
	
	for (i = 0; i < 31; i++) {
		ht1632c_shift_right();
	}
}

void ht1632c_slide_out_to_left(void) {
	
	uint8_t i;
	
	for (i = 0; i < 31; i++) {
		ht1632c_shift_left();
	}
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
	for (uint8_t i = bits; i > 0; i--) {
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

static inline void ht1632c_set_cursor(int16_t x, int16_t y) {
	cursor_x = x;
	cursor_y = y;
}

void ht1632c_draw_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size) {
	
	uint8_t i;
	uint8_t j;
	
	for (i = 0; i < 5; i++) {
		uint8_t line = pgm_read_byte(font_5x7+(c*5)+i);
		for (j = 7; j > 0; j--) {
			if (line & 0x1) {
				if (size == 1) {
					if (x+i >= 0) {
						ht1632c_draw_pixel(x+i, y+j-1, color);
					}
				}
				else {  // big size
					ht1632c_draw_filled_rect(x+i*size, y+j*size, size, size, color);
				}
			}
			line >>= 1;
		}
	}
}

void ht1632c_draw_char_small(int16_t x, int16_t y, char c,uint16_t color, uint8_t size) {
	
	int8_t i;
	int8_t j;
	
	for (i = 0; i < 6; i++) {
		uint8_t line = pgm_read_byte(font_4x6+(c*6)+i);
		for (j = 7; j > -1; j--) {
			if (line & 0x1) {
				if (size == 1) {
					if (x+j-2 >= 0) {
						ht1632c_draw_pixel(x+j-2, y-i+3, color);
					}
				}
				else {  // big size
					ht1632c_draw_filled_rect(x+j*size, y-i*size, size, size, color);
				}
			}
			line >>= 1;
		}
	}
}

void ht1632c_print(uint8_t *str, bool big_font) {
	
	ht1632c_clear_buffer();
	while(*str) {
		if (big_font) {
			ht1632c_draw_char(cursor_x, cursor_y, *str++, 1, textsize);
			cursor_x += textsize*6;
		} else {
			ht1632c_draw_char_small(cursor_x, cursor_y, *str++, 1, textsize);
			cursor_x += textsize*4;
		}
	}
	ht1632c_refresh_screen();
}

void ht1632c_scroll_print(uint8_t *str, bool big_font) {
	
	uint8_t y;
	int16_t i;
	int16_t length;
	
	if (big_font) {
		y = 5;
		length = strlen(str)*6;
	} else {
		y = 7;
		length = strlen(str)*4;
	}
	
	for (i = (WIDTH*2); i > -((int16_t)length); i--) {
		ht1632c_set_cursor(i,y);
		ht1632c_print(str,big_font);
		_delay_ms(80);
	}
}

void ht1632c_motion_print(uint8_t *str, int16_t x) {
	
	ht1632c_set_cursor(x,5);
	ht1632c_print(str,true);
	_delay_ms(15);
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

static inline void delay_ms( int ms )
{
	for (int i = 0; i < ms; i++)
	{
		_delay_ms(1);
	}
}

void ht1632c_loading(void) {
	uint8_t i;
	for (i = 0; i < WIDTH; i++) {
		ht1632c_set_brightness(i);
		ht1632c_draw_pixel(i-1,0,0);
		ht1632c_draw_pixel(i,0,1);
		ht1632c_refresh_screen();
		delay_ms(i*7);
	}
	for (i = WIDTH; i > 0; i--) {
		ht1632c_set_brightness(i);
		ht1632c_draw_pixel(i+1,0,0);
		ht1632c_draw_pixel(i,0,1);
		ht1632c_refresh_screen();
		delay_ms((WIDTH-i)*7);
	}
	ht1632c_clear_screen();
}

void ht1632c_shift_left(void) {
	
	int8_t i;
	
	for (i = 0; i <= 30; i += 2) {
		if (i == 14) {
			ledmatrix[14] = ledmatrix[1];
			} else if(i == 30) {
			ledmatrix[30] = ledmatrix[17];
			} else {
			ledmatrix[i] = ledmatrix[i+2];
		}
	}
	
	for (i = 1; i <= 31; i += 2) {
		if (i == 15) {
			ledmatrix[15] = 0;
		} else if (i == 31) {
			ledmatrix[31] = 0;
		} else {
			ledmatrix[i] = ledmatrix[i+2];
		}
	}
	
	ht1632c_refresh_screen();
	_delay_ms(10);
}

void ht1632c_shift_right(void) {
	
	int8_t i;
	
	for (i = 31; i > 0; i -= 2) {
		if (i == 1) {
			ledmatrix[1] = ledmatrix[14];
		} else if(i == 17) {
			ledmatrix[17] = ledmatrix[30];
		} else {
			ledmatrix[i] = ledmatrix[i-2];
		}
	}
	for (i = 30; i >= 0; i -= 2) {
		if (i == 0) {
			ledmatrix[0] = 0;
		} else if(i == 16) {
			ledmatrix[16] = 0;
		} else {
			ledmatrix[i] = ledmatrix[i-2];
		}
	}
	
	ht1632c_refresh_screen();
	_delay_ms(10);
	
}

void ht1632c_shift_right_in(void) {
	uint8_t ledmatrix_2[32];
	memset(ledmatrix_2,0,32);
	
	ledmatrix_2[31] = 0x0f;
	ledmatrix_2[30] = 0x07;
	ledmatrix_2[29] = 0x03;
	ledmatrix_2[28] = 0x01;
	
	int8_t i;
	
	for (i = 31; i > 0; i--)
	{
		ledmatrix[0] = ledmatrix_2[i];
		ht1632c_shift_right();
	}
	
}




void ht1632_dummy(void) {
	ht1632c_draw_char_small(1,7,'1',1,1);
	ht1632c_draw_char_small(5,7,'7',1,1);
	//ht1632c_draw_char_2(7,8,':',1,1);
	ht1632c_draw_char_small(10,7,'3',1,1);
	ht1632c_draw_char_small(14,7,'4',1,1);
	//ht1632c_refresh_screen();
}

void ht1632c_draw_button_info(void) {
	ht1632c_draw_pixel(1,15,1);
	ht1632c_draw_pixel(0,14,1);
	ht1632c_draw_pixel(1,14,1);
	ht1632c_draw_pixel(1,13,1);
	
	ht1632c_draw_pixel(14,15,1);
	ht1632c_draw_pixel(14,14,1);
	ht1632c_draw_pixel(15,14,1);
	ht1632c_draw_pixel(14,13,1);
	
	ht1632c_draw_pixel(4,15,1);
	ht1632c_draw_pixel(6,15,1);
	ht1632c_draw_pixel(5,14,1);
	ht1632c_draw_pixel(4,13,1);
	ht1632c_draw_pixel(6,13,1);
	
	ht1632c_draw_pixel(10,15,1);
	ht1632c_draw_pixel(9,14,1);
	ht1632c_draw_pixel(10,14,1);
	ht1632c_draw_pixel(11,14,1);
	ht1632c_draw_pixel(10,13,1);
	
	ht1632c_refresh_screen();
}