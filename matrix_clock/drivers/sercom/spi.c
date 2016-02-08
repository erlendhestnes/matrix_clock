/*
 * spi.c
 *
 * Created: 9/29/2015 9:26:18 PM
 *  Author: Administrator
 */ 

#include "spi.h"

void spi_setup(void) 
{
	//Disable power reduction for SPIC 
	PR.PRPC &= ~0x08;	
	
	PORTC.DIRSET = SD_CS | SD_MOSI | SD_SCK;
	PORTC.DIRCLR = SD_MISO;
	PORTC.OUTSET = SD_CS;
	
	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESCALER_DIV4_gc;
}

void spi_disable(void) 
{	
	SPIC.CTRL = 0;
	
	PORTC.DIRCLR   = SD_CS | SD_MOSI | SD_SCK;
	PORTC.PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTC.PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTC.PIN6CTRL = PORT_OPC_PULLUP_gc;
	PORTC.PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	//Enable power reduction for SPIC 
	PR.PRPC |= 0x08;
}

uint8_t spi_wr_rd(uint8_t data) 
{	
	uint16_t timeout = 0;
	
	SPIC.DATA = data;
	while(!(SPIC.STATUS & SPI_IF_bm)) {
		if (timeout++ == 0xffff) {
			return 0;
		}
	}
	return SPIC.DATA;
}
