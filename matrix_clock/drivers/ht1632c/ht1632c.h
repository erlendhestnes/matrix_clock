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

#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 16

//HT1632C FUNCTIONS
void ht1632c_setup(uint8_t type);
void ht1632c_power_down(void);
void ht1632c_set_brightness(int8_t pwm);
void ht1632c_blink(bool blink_on);
void ht1632c_set_pixel(uint16_t i);
void ht1632c_clr_pixel(uint16_t i);

//REFRESH FUNCTIONS
void ht1632c_refresh_screen(void);
void ht1632c_fill_screen(void);
void ht1632c_clear_screen(void);
void ht1632c_write_data(uint16_t data, uint8_t bits);
void ht1632c_write_ram(uint8_t addr, uint8_t data);
void ht1632c_write_command(uint8_t cmd);

//SHIFT FUNCTIONS
void ht1632c_shift_left(void);
void ht1632c_shift_right(void);
void ht1632c_shift_up(void);
void ht1632c_shift_down(void);

/*------------------------------SLIDE FUNCTIONS------------------------------*/

void ht1632c_slide_in_from_left(void);
void ht1632c_slide_in_from_right(void);
void ht1632c_slide_in_from_bottom(void);
void ht1632c_slide_in_from_top(void);

/*------------------------------PRINT FUNCTIONS------------------------------*/

void ht1632c_print_buffer(char *buffer, uint8_t length);

#endif /* HT1632_H_ */