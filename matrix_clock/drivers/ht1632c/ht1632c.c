/*
 * ht1632c.c
 *
 * Created: 10/9/2014 5:50:34 PM
 *  Author: Administrator
 */ 

#include "ht1632c.h"
#include "../rtc/rtc.h"
#include "../../4x6_font.c"
#include "../../modules/display/display.h"

#define swap(a, b) { uint16_t t = a; a = b; b = t; }

#define HT1632C_CS		PIN0_bm
#define HT1632C_RD		PIN1_bm
#define HT1632C_WR		PIN2_bm
#define HT1632C_DATA	PIN3_bm

/*--------------------------------SETUP-----------------------------------*/

void ht1632c_setup(uint8_t type) 
{
	PORTE.DIRSET = HT1632C_CS | HT1632C_WR | HT1632C_DATA;
	PORTE.OUTSET = HT1632C_CS | HT1632C_WR;
	
	/* READ - Not used
	HT1632_PORT.DIRSET = _rd;
	HT1632_PORT.OUTSET = _rd;
	*/

	ht1632c_write_command(HT1632_SYS_EN);
	ht1632c_write_command(HT1632_LED_ON);
	ht1632c_write_command(HT1632_BLINK_OFF);
	ht1632c_write_command(HT1632_MASTER_MODE);
	ht1632c_write_command(HT1632_INT_RC);
	ht1632c_write_command(type);
	ht1632c_write_command(HT1632_PWM_CONTROL | 0x1);
}

void ht1632c_power_down(void) 
{
	ht1632c_write_command(HT1632_LED_OFF);
	ht1632c_write_command(HT1632_SYS_DIS);
	
	PORTE.DIRCLR = HT1632C_CS | HT1632C_WR | HT1632C_RD | HT1632C_DATA;
	
	PORTE.PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTE.PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTE.PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTE.PIN3CTRL = PORT_OPC_PULLUP_gc;
}

void ht1632c_set_brightness(int8_t pwm) 
{
	if (pwm > 15) {
		pwm = 15;	
	} else if (pwm < 0) {
		pwm = 0;
	}
	//env_variables.brightness = pwm;
	ht1632c_write_command(HT1632_PWM_CONTROL | pwm);
}

void ht1632c_blink(bool blink_on) 
{	
	if (blink_on) {
		ht1632c_write_command(HT1632_BLINK_ON);	
	} else {
		ht1632c_write_command(HT1632_BLINK_OFF);	
	}
}

void ht1632c_set_pixel(uint16_t i) 
{	
	ledmatrix[i/8] |= _BV(i%8);
}

void ht1632c_clr_pixel(uint16_t i) 
{	
	ledmatrix[i/8] &= ~_BV(i%8);
}

/*-------------------------REFRESH FUNCTIONS-----------------------------------*/

void ht1632c_fill_screen(void) 
{
	for (uint8_t i = 0; i < (DISPLAY_WIDTH*DISPLAY_HEIGHT/8); i++) {
		ledmatrix[i] = 0xFF;
	}
	display_refresh_screen();
}

void ht1632c_clear_screen(void) 
{
	for (uint8_t i = 0; i < (DISPLAY_WIDTH*DISPLAY_HEIGHT/8); i++) {
		ledmatrix[i] = 0;
	}
	display_refresh_screen();
}

void ht1632c_refresh_screen(void) 
{
	PORTE.OUTCLR = HT1632C_CS;

	ht1632c_write_data(HT1632_WRITE, 3);
	// send with address 0
	ht1632c_write_data(0, 7);

	for (uint16_t i = 0; i < 32; i += 2) {
		uint16_t d = ledmatrix[i];
		d <<= 8;
		d |= ledmatrix[i+1];

		ht1632c_write_data(d, 16);
	}
	PORTE.OUTSET = HT1632C_CS;
}

void ht1632c_clear_buffer(void) 
{
	for (uint8_t i = 0; i < (DISPLAY_WIDTH*DISPLAY_HEIGHT/8); i++) {
		ledmatrix[i] = 0;
	}
}

void ht1632c_write_data(uint16_t data, uint8_t bits) 
{
	for (uint8_t i = bits; i > 0; i--) {
		PORTE.OUTCLR = HT1632C_WR;
		if (data & _BV(i-1)) {
			PORTE.OUTSET = HT1632C_DATA;
		} else {
			PORTE.OUTCLR = HT1632C_DATA;
		}
		PORTE.OUTSET = HT1632C_WR;
	}
}

void ht1632c_write_ram(uint8_t addr, uint8_t data) 
{
	uint16_t d = HT1632_WRITE;
	d <<= 7;
	d |= addr & 0x7F;
	d <<= 4;
	d |= data & 0xF;
		
	PORTE.OUTCLR = HT1632C_CS;
	ht1632c_write_data(d, 14);
	PORTE.OUTSET = HT1632C_CS;
}

void ht1632c_write_command(uint8_t cmd) 
{
	uint16_t data = 0;
	data = HT1632_COMMAND;
	data <<= 8;
	data |= cmd;
	data <<= 1;
		
	PORTE.OUTCLR = HT1632C_CS;
	ht1632c_write_data(data, 12);
	PORTE.OUTSET = HT1632C_CS;
}


/*------------------------------SHIFT FUNCTIONS------------------------------*/

void ht1632c_shift_left(void) 
{
	int8_t i;
	
	display_refresh_screen();
	_delay_ms(10);
	
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
	
	/*
	//Why does not this work??
	for (i = 0; i <= 31; i++) {
		if (i == 14) {
			ledmatrix[14] = ledmatrix[1];
		} else if (i == 15) {
			ledmatrix[15] = 0;
		} else if(i == 30) {
			ledmatrix[30] = ledmatrix[17];
		} else if (i == 31) {
			ledmatrix[31] = 0;
		} else {
			ledmatrix[i] = ledmatrix[i+2];
		}
	}
	*/
}

void ht1632c_shift_right(void) 
{	
	int8_t i;
	
	display_refresh_screen();
	_delay_ms(10);
	
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
	/*
	for (i = 31; i > 0; i--) {
		if(i == 17) {
			ledmatrix[17] = ledmatrix[30];
		} else if(i == 16) {
			ledmatrix[16] = 0;
		} else if (i == 1) {
			ledmatrix[1] = ledmatrix[14];
		} else if (i == 0) {
			ledmatrix[0] = 0;
		} else {
			ledmatrix[i] = ledmatrix[i-2];
		}
	}
	*/
}

void ht1632c_shift_up(void) 
{
	uint8_t temp_top;
	uint8_t temp_bottom;
	uint16_t temp;
	
	uint8_t i;
	
	for (i = 0; i < 15; i++) {
		temp_top = ledmatrix[i];
		temp_bottom = ledmatrix[i+16];
		temp = (temp_top << 8) | (temp_bottom);
		temp <<= 1;
		ledmatrix[i] = (temp >> 8);
		ledmatrix[i+16] = temp;
	}
	
	display_refresh_screen();
	_delay_ms(10);
}

void ht1632c_shift_down(void) 
{
	uint8_t temp_top;
	uint8_t temp_bottom;
	uint16_t temp;
	
	uint8_t i;
	
	for (i = 0; i < 15; i++) {
		temp_top = ledmatrix[i];
		temp_bottom = ledmatrix[i+16];
		temp = (temp_top << 8) | (temp_bottom);
		temp >>= 1;
		ledmatrix[i] = (temp >> 8);
		ledmatrix[i+16] = temp;
	}
	
	display_refresh_screen();
	_delay_ms(10);
}

/*------------------------------SLIDE FUNCTIONS------------------------------*/

void ht1632c_slide_in_from_left(void) 
{
	uint8_t temp[32];
	
	memset(temp,0,32);
	memcpy(temp,ledmatrix,32);
	memset(ledmatrix,0,32);
	
	int8_t i;
	
	for (i = 31; i >= 17; i -= 2) {
		ledmatrix[0] = temp[i-16];
		ledmatrix[16] = temp[i];
		ht1632c_shift_right();
	}
	
	for (i = 30; i >= 16; i -= 2) {
		ledmatrix[0] = temp[i-16];
		ledmatrix[16] = temp[i];
		if (i == 16) {
			display_refresh_screen();
		} else {
			ht1632c_shift_right();
		}
	}
}

void ht1632c_slide_in_from_right(void) 
{
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
	
	for (i = 1; i <= 15; i += 2)
	{
		ledmatrix[15] = temp[i];
		ledmatrix[31] = temp[i+16];
	
		if (i == 15) {
			display_refresh_screen();
		} else {
			ht1632c_shift_left();
		}
	}
}

void ht1632c_slide_in_from_bottom(void) 
{
	uint8_t temp[32];
	
	memset(temp,0,32);
	memcpy(temp,ledmatrix,32);
	memset(ledmatrix,0,32);
	
	uint8_t col;
	uint8_t row;
	
	for (row = 0; row < 16; row++) {
		for (col = 0; col < 15; col++) {
			if (row < 8) {
				ledmatrix[col+16] |= (temp[col] & (0x80 >> row)) >> (7-row);
				} else {
				ledmatrix[col+16] |= (temp[col+16] & (0x80 >> (row-8))) >> (15-row);
			}
		}
		if (row < 15) {
			ht1632c_shift_up();
			} else {
			display_refresh_screen();
		}
	}
}

void ht1632c_slide_in_from_top(void) 
{
	uint8_t temp[32];
	
	memset(temp,0,32);
	memcpy(temp,ledmatrix,32);
	memset(ledmatrix,0,32);
	
	int8_t col;
	int8_t row;
	
	for (row = 15; row > -1; row--) {
		for (col = 0; col < 15; col++) {
			if (row > 7) {
				ledmatrix[col] |= (temp[col+16] & (0x01 << (15-row))) << (row-8);
			} else {
				ledmatrix[col] |= (temp[col] & (0x01 << (7-row))) << (row);
			}
		}
		if (row > 0) {
			ht1632c_shift_down();
		} else {
			display_refresh_screen();
		}
	}
}

/*------------------------------PRINT FUNCTIONS------------------------------*/

void ht1632c_print_buffer(char *buffer, uint16_t length) 
{
	uint16_t i;
	
	//Shift buffer into visible area
	for (i = 0; i < (length); i++) {
		ht1632c_shift_left();
		ledmatrix[15] = buffer[i] >> 3;
		ledmatrix[31] = buffer[i] << 5;
		_delay_ms(25);
	}
	
	//Shift buffer out of visible area
	if ((length/4) < 15) {
		for (i = 0; i < 30; i++) {
			ht1632c_shift_left();
			_delay_ms(25);
		}
	} else {
		for (i = 0; i < (length/4); i++) {
			ht1632c_shift_left();
			_delay_ms(25);
		}
	}
}