/*
 * ht1632c.h
 *
 * Created: 10/9/2014 5:50:45 PM
 *  Author: Administrator
 */ 

#ifndef HT1632_H_
#define HT1632_H_

#include <avr/io.h>

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

void ht1632c_begin(uint8_t type);
void ht1632c_powerdown(void);
void ht1632c_setBrightness(uint8_t pwm);
void ht1632c_blink(uint8_t blinky);
void ht1632c_drawBitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color);
void ht1632c_drawPixel(uint8_t x, uint8_t y, uint8_t color);
void ht1632c_setPixel(uint16_t i);
void ht1632c_clrPixel(uint16_t i);
void ht1632c_writeScreen();
void ht1632c_clearScreen();
void ht1632c_writedata(uint16_t d, uint8_t bits);
void ht1632c_writeRAM(uint8_t addr, uint8_t data);
void ht1632c_sendcommand(uint8_t cmd);
void ht1632c_fillScreen();
void ht1632c_drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1,uint8_t color);
void ht1632c_drawCircle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color);
void ht1632c_fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void ht1632c_drawChar(uint8_t x, uint8_t y, char c,uint16_t color, uint8_t size);
void ht1632c_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color);
void ht1632c_write_char(uint8_t base,uint8_t character);

void traverse_screen(void);
void traverse_screen_clear(void);
void pwm_fade(void);
void ram_test(void);
void fade_up(void);
void fade_down(void);

void ht1632c_setCursor(uint8_t x, uint8_t y);
void ht1632c_write(uint8_t c);
void ht1632c_print(uint8_t *str);
void ht1632_random(void);

#endif /* HT1632_H_ */