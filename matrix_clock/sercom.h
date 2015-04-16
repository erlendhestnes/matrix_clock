/*
 * spi.h
 *
 * Created: 12/13/2014 5:52:47 PM
 *  Author: Administrator
 */ 

#ifndef _SERCOM_H_
#define _SERCOM_H_

#include <avr/io.h>
#include <stdint.h>

#define SD_PORT		PORTC
#define SD_SPI		SPIC

#define SD_CD		PIN3_bm
#define SD_CS		PIN4_bm
#define SD_MOSI		PIN5_bm
#define SD_MISO		PIN6_bm
#define SD_SCK		PIN7_bm

#define MAX44007_ADDR_0  0x94
#define MAX44007_READ    0b10110101
#define MAX44007_WRITE   0b10110100

#define LUX_LOW			0x04
#define LUX_HIGH		0x03

void spi_on(void);
void spi_off(void);
uint8_t spi_wr_rd(uint8_t spi_data);

static inline void ss_select(void) {
	SD_PORT.OUTCLR = SD_CS;
}

static inline void ss_deselect(void) {
	SD_PORT.OUTSET = SD_CS;
}

void i2c_setup(void);

uint8_t i2c_write_data(uint8_t slave_addr, uint8_t register_addr, uint8_t data);
uint8_t i2c_read_data(uint8_t slave_addr, uint8_t register_addr);

uint8_t i2c_write_data_block(uint8_t slave_addr, uint8_t register_addr, uint8_t *data, uint8_t length);
uint8_t i2c_read_data_block(uint8_t slave_addr, uint8_t register_addr, uint8_t *data, uint8_t length);

#endif