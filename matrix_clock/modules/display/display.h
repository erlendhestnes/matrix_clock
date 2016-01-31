/*
 * display.h
 *
 * Created: 9/29/2015 5:53:47 PM
 *  Author: Administrator
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "../../global.h"
#include "../../drivers/ht1632c/ht1632c.h"

void display_setup(void);
void display_fill_screen(void);
void display_clear_screen(void);
void display_refresh_screen(void);
void display_on(void);
void display_off(void);

//FADE FUNCTIONS
void display_fade(uint8_t pwm);
void display_fade_blink(void);

//LOADING FUNCTIONS
void display_show_loading_bar(void);
void display_show_loading_square(void);
void display_show_loading_square_large(void);

//GEOMETRIC FUNCTIONS
void display_draw_pixel(int x, int y, uint8_t color);
void display_draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color);
void display_draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color);
void display_draw_circle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color);
void display_draw_filled_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void display_draw_bitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color);

//PRINT FUNCTIONS
void display_print_scrolling_text(const char *str, bool big_font);
void display_print_buffer(char *buffer, uint16_t length);
void display_draw_char_to_buffer(int16_t x, char c, char *buffer);
void display_draw_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size);
void display_draw_small_char_to_buffer(int16_t x, char c, char *buffer);
void display_draw_small_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size);
void display_draw_four_letter_word(const char *name);
void display_draw_three_letter_word(const char *name);

//MISC FUNCTIONS
void display_traverse_screen(bool color);
void display_draw_button_info(void);
void display_draw_wifi_icon(void);

//SLIDE FUNCTIONS
void display_slide_in_from_left(void);
void display_slide_in_from_right(void);
void display_slide_in_from_bottom(void);
void display_slide_in_from_top(void);
void display_slide_out_to_right(void);
void display_slide_out_to_left(void);
void display_slide_out_to_top(void);
void display_slide_out_to_bottom(void);

//TIME FUNCTIONS
void display_alarm_increment_minute(void);
void display_alarm_decrement_minute(void);
void display_alarm_increment_hour(void);
void display_alarm_decrement_hour(void);

void display_draw_and_increment_hour(void);
void display_draw_and_decrement_hour(void);
void display_draw_and_increment_minute(void);
void display_draw_and_decrement_minute(void);
void display_draw_and_increment_second(void);
void display_draw_and_decrement_second(void);
void display_draw_and_increment_day(void);
void display_draw_and_decrement_day(void);
void display_draw_and_increment_month(void);
void display_draw_and_decrement_month(void);
void display_draw_and_increment_year(void);
void display_draw_and_decrement_year(void);

#endif /* DISPLAY_H_ */