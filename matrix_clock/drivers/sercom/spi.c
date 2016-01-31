/*
 * spi.c
 *
 * Created: 9/29/2015 9:26:18 PM
 *  Author: Administrator
 */ 

#include "spi.h"

void spi_setup(void) 
{	
	SD_PORT.DIRSET = SD_CS | SD_MOSI | SD_SCK;
	SD_PORT.OUTSET = SD_CS;
	SD_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESCALER_DIV4_gc;
}

void spi_off(void) 
{	
	SD_SPI.CTRL &= ~(SPI_ENABLE_bm);
	SD_PORT.DIRCLR = SD_CS | SD_MOSI | SD_SCK;
	SD_PORT.PIN4CTRL = PORT_OPC_PULLUP_gc;
	SD_PORT.PIN5CTRL = PORT_OPC_PULLUP_gc;
	SD_PORT.PIN6CTRL = PORT_OPC_PULLUP_gc;
	SD_PORT.PIN7CTRL = PORT_OPC_PULLUP_gc;
}

uint8_t spi_wr_rd(uint8_t data) 
{	
	SD_SPI.DATA = data;
	//Warning: this may hang
	while(!(SPIC.STATUS & SPI_IF_bm));
	return SD_SPI.DATA;
}
