/*
 * display.c
 *
 * Created: 9/29/2015 5:53:38 PM
 *  Author: Administrator
 */ 

#include "display.h"
#include "../../4x6_font.c"

void display_setup(void) 
{
	ht1632c_setup(HT1632_COMMON_16NMOS);
	ht1632c_set_brightness(1);
	display_clear_screen();
	ht1632c_write_command(HT1632_LED_ON);
}

void display_fill_screen(void) 
{
	ht1632c_fill_screen();
}

void display_clear_screen(void) 
{
	ht1632c_clear_screen();
}

void display_refresh_screen(void) 
{
	ht1632c_refresh_screen();
}

/*------------------------------FADE FUNCTIONS---------------------------*/

static inline void display_fade_up(uint8_t pwm, uint8_t prev_pwm) 
{
	while (prev_pwm < pwm) {
		ht1632c_set_brightness(prev_pwm);
		_delay_ms(15);
		prev_pwm++;
	}
}

static inline void display_fade_down(uint8_t pwm, uint8_t prev_pwm) 
{
	while (prev_pwm > pwm) {
		ht1632c_set_brightness(prev_pwm);
		_delay_ms(15);
		prev_pwm--;
	}
}

void display_fade(uint8_t pwm) 
{
	static uint8_t prev_pwm = 1;
	
	if (pwm > prev_pwm) {
		display_fade_up(pwm, prev_pwm);
	} else if (pwm < prev_pwm) {
		display_fade_down(pwm, prev_pwm);
	} else  {
		//do nothing
	}
	prev_pwm = pwm;
}

void display_fade_blink(void) 
{
	display_fade(1);
	display_fade(15);
	display_fade(1);
	display_fade(15);
}

/*------------------------------LOADING FUNCTIONS---------------------------*/

void display_show_loading_bar(void) 
{
	uint8_t i;
	
	for (i = 0; i < DISPLAY_WIDTH; i++) {
		ht1632c_set_brightness(i);
		display_draw_pixel(i-1,0,0);
		display_draw_pixel(i,0,1);
		display_refresh_screen();
		delay_ms(i*7);
	}
	for (i = DISPLAY_WIDTH; i > 0; i--) {
		ht1632c_set_brightness(i);
		display_draw_pixel(i+1,0,0);
		display_draw_pixel(i,0,1);
		display_refresh_screen();
		delay_ms((DISPLAY_WIDTH-i)*7);
	}
	display_clear_screen();
}

void display_show_loading_square(void) 
{
	static uint8_t x = 9;
	static uint8_t y = 9;
	static uint8_t color = 1;
	
	display_draw_filled_rect(x,y,1,1,1);
	
	if (y > 6 && x == 9) {
		y--;
		} else if (y == 6 && x > 6) {
		x--;
		} else if (y < 9 && x == 6) {
		y++;
		} else if (y == 9 && x < 9) {
		x++;
		if (x == 9)
		color ^= 1;
	}
	display_draw_filled_rect(x,y,1,1,0);
	display_refresh_screen();
}

void display_show_loading_square_large(void) 
{
	static uint8_t x = 10;
	static uint8_t y = 10;
	static uint8_t color = 1;
	
	display_draw_filled_rect(x,y,1,1,0);
	
	if (x > 5 && y == 10) {
		x--;
	} else if (x == 5 && y > 5) {
		y--;
	} else if (x < 10 && y == 5) {
		x++;
	} else if (x == 10 && y < 10) {
		y++;
	if (y == 10)
		color ^= 1;
	}
	display_draw_filled_rect(x,y,1,1,1);
	display_refresh_screen();

}

/*------------------------------GEOMETRIC FUNCTIONS---------------------------*/

void display_draw_pixel(int x, int y, uint8_t color) 
{
	if (x >= DISPLAY_WIDTH) {
		return;
	}
	if (y >= DISPLAY_HEIGHT) {
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

	if (color) {
		ht1632c_set_pixel(i);
	} else {
		ht1632c_clr_pixel(i);	
	}
}

void display_draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color) 
{
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
			display_draw_pixel(y0, x0, color);
			} else {
			display_draw_pixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void display_draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color) 
{
	uint8_t i;
	uint8_t j;
	
	for (i = x; i < x + w; i++) {
		for (j = y; j < y + h; j++) {
			display_draw_pixel(i, j, color);
		}
	}
}

void display_draw_circle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color) 
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	display_draw_pixel(x0, y0+r, color);
	display_draw_pixel(x0, y0-r, color);
	display_draw_pixel(x0+r, y0, color);
	display_draw_pixel(x0-r, y0, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		display_draw_pixel(x0 + x, y0 + y, color);
		display_draw_pixel(x0 - x, y0 + y, color);
		display_draw_pixel(x0 + x, y0 - y, color);
		display_draw_pixel(x0 - x, y0 - y, color);
		
		display_draw_pixel(x0 + y, y0 + x, color);
		display_draw_pixel(x0 - y, y0 + x, color);
		display_draw_pixel(x0 + y, y0 - x, color);
		display_draw_pixel(x0 - y, y0 - x, color);
		
	}
}

void display_draw_filled_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) 
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	display_draw_line(x0, y0-r, x0, y0+r+1, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		display_draw_line(x0+x, y0-y, x0+x, y0+y+1, color);
		display_draw_line(x0-x, y0-y, x0-x, y0+y+1, color);
		display_draw_line(x0+y, y0-x, x0+y, y0+x+1, color);
		display_draw_line(x0-y, y0-x, x0-y, y0+x+1, color);
	}
}

void display_draw_bitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color) 
{
	uint8_t i;
	uint8_t j;
	
	for (j=0; j<h; j++) {
		for (i=0; i<w; i++ ) {
			if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) {
				display_draw_pixel(x+i, y+j, color);
			}
		}
	}
}

/*------------------------------PRINT FUNCTIONS------------------------------*/

void display_print_scrolling_text(char *str, bool big_font) 
{
	uint16_t i = 0;
	uint16_t length;
	
	//Calculate length of buffer based on string size
	if (big_font) {
		length = strlen(str)*6;
	} else {
		length = strlen(str)*4;
	}
	
	//This is a potential stack overflow...
	char *buffer;
	buffer = (char *) calloc(length,sizeof(char));
	//char buffer[100];
	//memset(buffer,0,100);
	
	//Fill buffer with text
	while(*str) {
		if (big_font) {
			display_draw_char_to_buffer(6*(i++), *str++, buffer);
		} else {
			display_draw_small_char_to_buffer(4*(i++), *str++, buffer);
		}
	}
	
	display_print_buffer(buffer, length);
	
	free(buffer);
}

void display_print_buffer(char *buffer, uint8_t length)
{
	ht1632c_print_buffer(buffer,length);
}

void display_draw_char_to_buffer(int16_t x, char c, char *buffer) 
{
	uint8_t i;
	uint16_t d = c*5;
	
	for (i = 0; i < 5; i++) {
		char line = pgm_read_byte(font_5x7+d+i);
		buffer[x+i] = reverse_byte(line);
	}
}

void display_draw_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size) 
{
	uint8_t i;
	uint8_t j;
	
	for (i = 0; i < 5; i++) {
		uint8_t line = pgm_read_byte(font_5x7+(c*5)+i);
		for (j = 7; j > 0; j--) {
			if (line & 0x1) {
				if (size == 1) {
					if (x+i >= 0) {
						display_draw_pixel(x+i, y+j-1, color);
					}
				}
				else {  // big size
					display_draw_filled_rect(x+i*size, y+j*size, size, size, color);
				}
			}
			line >>= 1;
		}
	}
}

void display_draw_small_char_to_buffer(int16_t x, char c, char *buffer) 
{	
	uint16_t i;
	uint16_t j;
	uint16_t d = c*6;
	char temp[5];
	
	for (j = 0; j < 5; j++) {
		char line = pgm_read_byte(font_4x6+d+j);
		temp[j] = (line >> 4);
		for (i = 0; i < 3; i++) {
			buffer[(2-i)+x] |= (temp[j] & 0x01) << (5-j);
			temp[j] >>= 1;
		}
	}
}

void display_draw_small_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size) 
{	
	int8_t i;
	int8_t j;
	
	for (i = 0; i < 6; i++) {
		uint8_t line = pgm_read_byte(font_4x6+(c*6)+i);
		for (j = 7; j > -1; j--) {
			if (line & 0x1) {
				if (size == 1) {
					if (x+j-2 >= 0) {
						display_draw_pixel(x+j-2, y-i+3, color);
					}
				}
				else {  // big size
					display_draw_filled_rect(x+j*size, y-i*size, size, size, color);
				}
			}
			line >>= 1;
		}
	}
}

void display_draw_four_letter_word(char *name)
{
	display_draw_small_char(1,7,*name++,1,1);
	display_draw_small_char(5,7,*name++,1,1);
	display_draw_small_char(9,7,*name++,1,1);
	display_draw_small_char(13,7,*name++,1,1);
}

void display_draw_three_letter_word(char *name)
{
	display_draw_small_char(3,7,*name++,1,1);
	display_draw_small_char(7,7,*name++,1,1);
	display_draw_small_char(11,7,*name++,1,1);
}

/*------------------------------MISC FUNCTIONS------------------------------*/

//Nice functions to test for "dead" pixels
void display_traverse_screen(bool color)
{
	static uint8_t x = 1;
	static uint8_t y = 1;
	
	display_draw_pixel(x,y,color);
	display_refresh_screen();
	x++;
	y += x / 16;
	x %= 16;
	y %= 16;
}

void display_draw_button_info(void)
{
	display_draw_pixel(1,15,1);
	display_draw_pixel(0,14,1);
	display_draw_pixel(1,14,1);
	display_draw_pixel(1,13,1);
	
	display_draw_pixel(14,15,1);
	display_draw_pixel(14,14,1);
	display_draw_pixel(15,14,1);
	display_draw_pixel(14,13,1);
	
	display_draw_pixel(4,15,1);
	display_draw_pixel(6,15,1);
	display_draw_pixel(5,14,1);
	display_draw_pixel(4,13,1);
	display_draw_pixel(6,13,1);
	
	display_draw_pixel(10,15,1);
	display_draw_pixel(9,14,1);
	display_draw_pixel(10,14,1);
	display_draw_pixel(11,14,1);
	display_draw_pixel(10,13,1);
	
	display_refresh_screen();
}

/*------------------------------SLIDE FUNCTIONS------------------------------*/

void display_slide_in_from_left(void) 
{
	ht1632c_slide_in_from_left();
}

void display_slide_in_from_right(void) 
{	
	ht1632c_slide_in_from_right();
}

void display_slide_in_from_bottom(void) 
{
	ht1632c_slide_in_from_bottom();
}

void display_slide_in_from_top(void) 
{
	ht1632c_slide_in_from_top();
}

void display_slide_out_to_right(void) 
{
	uint8_t i;
	
	for (i = 0; i < 31; i++) {
		ht1632c_shift_right();
	}
}

void display_slide_out_to_left(void) 
{
	uint8_t i;
	
	for (i = 0; i < 31; i++) {
		ht1632c_shift_left();
	}
}

void display_slide_out_to_top(void) 
{
	uint8_t i;
	
	for (i = 0; i < 31; i++) {
		ht1632c_shift_up();
	}
}

void display_slide_out_to_bottom(void) 
{
	uint8_t i;
	
	for (i = 0; i < 31; i++) {
		ht1632c_shift_down();
	}
}

/*------------------------------TIME FUNCTIONS------------------------------*/

void display_alarm_increment_minute(uint8_t min) 
{
	if (min < 59) {
		rtc_update_display(5,min);
	} else {
		min = 0;
		rtc_update_display(5,min);
	}
	display_refresh_screen();
}

void display_alarm_decrement_minute(uint8_t min) 
{
	if (min > 0) {
		rtc_update_display(5,min);
	} else {
		min = 59;
		rtc_update_display(5,min);
	}
	display_refresh_screen();
}

void display_alarm_increment_hour(uint8_t hour) 
{
	if (hour < 24) {
		rtc_update_display(5,hour);
	} else {
		hour = 0;
		rtc_update_display(5,hour);
	}
	display_refresh_screen();
}

void display_alarm_decrement_hour(uint8_t hour) 
{
	if (hour > 0) {
		rtc_update_display(5,hour);
	} else {
		hour = 23;
		rtc_update_display(5,hour);
	}
	display_refresh_screen();
}

void display_draw_and_increment_hour(void) 
{
	if (env_var.time.hours < 59) {
		rtc_update_display(5,++env_var.time.hours);
	} else {
		env_var.time.hours = 0;
		rtc_update_display(5,env_var.time.hours);
	}
	display_refresh_screen();
}

void display_draw_and_decrement_hour(void) 
{
	if (env_var.time.hours > 0) {
		rtc_update_display(5,--env_var.time.hours);
	} else {
		env_var.time.hours = 59;
		rtc_update_display(5,env_var.time.hours);
	}
	display_refresh_screen();
}

void display_draw_and_increment_minute(void) 
{
	if (env_var.time.minutes < 59) {
		rtc_update_display(5,++env_var.time.minutes);
	} else {
		env_var.time.minutes = 0;
		rtc_update_display(5,env_var.time.minutes);
	}
	display_refresh_screen();
}

void display_draw_and_decrement_minute(void) 
{
	if (env_var.time.minutes > 0) {
		rtc_update_display(5,--env_var.time.minutes);
	} else {
		env_var.time.minutes = 59;
		rtc_update_display(5,env_var.time.minutes);
	}
	display_refresh_screen();
}

void display_draw_and_increment_second(void) 
{
	if (env_var.time.seconds < 59) {
		rtc_update_display(5,++env_var.time.seconds);
	} else {
		env_var.time.seconds = 0;
		rtc_update_display(5,env_var.time.seconds);
	}
	display_refresh_screen();
}

void display_draw_and_decrement_second(void) 
{
	if (env_var.time.seconds > 0) {
		rtc_update_display(5,--env_var.time.seconds);
	} else {
		env_var.time.seconds = 59;
		rtc_update_display(5,env_var.time.seconds);
	}
	display_refresh_screen();
}

void display_draw_and_increment_day(void) 
{
	if (env_var.time.weekday <= Sunday) {
		display_draw_three_letter_word(time_get_day_name(env_var.time.weekday++));
	} else {
		env_var.time.weekday = Monday;
		display_draw_three_letter_word(time_get_day_name(Monday));
	}
	display_refresh_screen();
}

void display_draw_and_decrement_day(void) 
{
	if (env_var.time.weekday > Monday) {
		display_draw_three_letter_word(time_get_day_name(env_var.time.weekday--));
	} else {
		env_var.time.weekday = Sunday;
		display_draw_three_letter_word(time_get_day_name(Sunday));
	}
	display_refresh_screen();
}

void display_draw_and_increment_month(void) 
{
	if (env_var.time.month <= December) {
		display_draw_three_letter_word(time_get_month_name(env_var.time.month++));
	} else {
		env_var.time.month = January;
		display_draw_three_letter_word(time_get_month_name(January));
	}
	display_refresh_screen();
}

void display_draw_and_decrement_month(void) 
{
	if (env_var.time.month > January) {
		display_draw_three_letter_word(time_get_month_name(env_var.time.month--));
	} else {
		env_var.time.month = December;
		display_draw_three_letter_word(time_get_month_name(December));
	}
	display_refresh_screen();
}

void display_draw_and_increment_year(void) 
{
	char *year;
	itoa_simple(year,env_var.time.year++);
	display_draw_four_letter_word(year);
	display_refresh_screen();
}

void display_draw_and_decrement_year(void) 
{
	char *year;
	itoa_simple(year,env_var.time.year--);
	display_draw_four_letter_word(year);
	display_refresh_screen();
}