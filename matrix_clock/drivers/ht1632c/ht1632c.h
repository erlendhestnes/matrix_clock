/*
 * ht1632c.h
 *
 * Created: 10/9/2014 5:50:45 PM
 *  Author: Administrator
 */ 

#ifndef _HT1632_H_
#define _HT1632_H_

#include "../../global.h"

#define HT1632_READ  0x6
#define HT1632_WRITE 0x5
#define HT1632_COMMAND 0x4

#define HT1632_SYS_DIS 0x00
#define HT1632_SYS_EN 0x01
#define HT1632_LED_OFF 0x02
#define HT1632_LED_ON 0x03
#define HT1632_BLINK_OFF 0x08
#define HT1632_BLINK_ON 0x09
#define HT1632_SLAVE_MODE 0x10
#define HT1632_MASTER_MODE 0x14
#define HT1632_INT_RC 0x18
#define HT1632_EXT_CLK 0x1C
#define HT1632_PWM_CONTROL 0xA0

#define HT1632_COMMON_8NMOS  0x20
#define HT1632_COMMON_16NMOS  0x24
#define HT1632_COMMON_8PMOS  0x28
#define HT1632_COMMON_16PMOS  0x2C

#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 15

void ht1632c_setup(uint8_t type);
void ht1632c_powerdown(void);
void ht1632c_set_brightness(int8_t pwm);
void ht1632c_blink(uint8_t blinky);
void ht1632c_write_data(uint16_t d, uint8_t bits);
void ht1632c_write_ram(uint8_t addr, uint8_t data);
void ht1632c_send_command(uint8_t cmd);

void ht1632c_clear_screen();
void ht1632c_fill_screen();
void ht1632c_refresh_screen();

void ht1632c_draw_pixel(int x, int y, uint8_t color);
void ht1632c_draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color);
void ht1632c_draw_circle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color);
void ht1632c_draw_filled_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void ht1632c_draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color);
void ht1632c_draw_bitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color);

void ht1632c_draw_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size);
void ht1632c_print(char *str, bool big_font);
void ht1632c_scroll_print(char *str, bool big_font);
void ht1632c_motion_print(char *str, int16_t x);

void ht1632c_shift_left(void);
void ht1632c_shift_right(void);
void ht1632c_shift_up(void);
void ht1632c_shift_down(void);

void ht1632c_slide_out_to_left(void);
void ht1632c_slide_out_to_right(void);
void ht1632c_slide_out_to_top(void);
void ht1632c_slide_out_to_bottom(void);

void ht1632c_print_buffer(char *buffer, uint8_t length);
void ht1632c_slide_in_from_left(void);
void ht1632c_slide_in_from_right(void);
void ht1632c_slide_in_from_top(void);
void ht1632c_slide_in_from_bottom(void);

void traverse_screen(void);
void traverse_screen_clear(void);

void ht1632c_fade(uint8_t pwm);
void ht1632_random(void);
void ht1632c_loading(void);
void ht1632c_loading2(void);
void ht1632c_loading3(void);
void ht1632c_draw_char_small(int16_t x, int16_t y, char c,uint16_t color, uint8_t size);

void ht1632_dummy(void);
void ht1632c_draw_button_info(void);

void ht1632c_draw_three_letter_word(char *name);
void ht1632c_draw_four_letter_word(char *name);
void ht1632c_fade_blink(void);

void ht1632c_increment_minute(uint8_t min);
void ht1632c_decrement_minute(uint8_t min);
void ht1632c_increment_hour(uint8_t hour);
void ht1632c_decrement_hour(uint8_t hour);

#endif /* HT1632_H_ */