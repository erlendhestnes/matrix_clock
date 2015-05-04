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

void ht1632c_setup(uint8_t type);
void ht1632c_powerdown(void);
void ht1632c_set_brightness(uint8_t pwm);
void ht1632c_blink(uint8_t blinky);
void ht1632c_draw_bitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color);
void ht1632c_draw_pixel(int x, int y, uint8_t color);
void ht1632c_set_pixel(uint16_t i);
void ht1632c_clr_pixel(uint16_t i);
void ht1632c_refresh_screen();
void ht1632c_clear_screen();
void ht1632c_write_data(uint16_t d, uint8_t bits);
void ht1632c_write_ram(uint8_t addr, uint8_t data);
void ht1632c_send_command(uint8_t cmd);
void ht1632c_fill_screen();
void ht1632c_draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color);
void ht1632c_draw_circle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color);
void ht1632c_fill_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void ht1632c_draw_char(int16_t x, int16_t y, char c,uint16_t color, uint8_t size);
void ht1632c_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color);
void ht1632c_write_char(uint8_t base,uint8_t character);

void traverse_screen(void);
void traverse_screen_clear(void);
void ht1632_fade(uint8_t pwm);

void ht1632c_set_cursor(int16_t x, int16_t y);
void ht1632c_write(uint8_t c);
void ht1632c_print(uint8_t *str);
void ht1632c_scroll_print(uint8_t *str, uint16_t len, uint16_t delay);
void ht1632c_motion_print(uint8_t *str, int16_t x);
void ht1632_random(void);
void ht1632c_loading(void);
void ht1632c_draw_char_2(int16_t x, int16_t y, char c,uint16_t color, uint8_t size);
void ht1632c_shift_left(void);
#endif /* HT1632_H_ */