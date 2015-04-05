/*
 * spi.c
 *
 * Created: 12/13/2014 5:52:36 PM
 *  Author: Administrator
 */ 

#include "sercom.h"
#include <util/delay.h>

void spi_on(void) {
	
	PORTC.DIRSET = CS | MOSI | SCK;	
	PORTC.OUTSET = CS;	
	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESCALER_DIV4_gc;
}

void spi_off(void) {
	
	SPIC.CTRL &= ~(SPI_ENABLE_bm);
	PORTC.DIRCLR = CS | MOSI | SCK;
	PORTC.PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTC.PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTC.PIN6CTRL = PORT_OPC_PULLUP_gc;
	PORTC.PIN7CTRL = PORT_OPC_PULLUP_gc;
}

uint8_t spi_wr_rd(uint8_t spi_data) {
	
	SPIC.DATA = spi_data;
	while(!(SPIC.STATUS & SPI_IF_bm));
	return SPIC.DATA;
}

void i2c_setup(void) {
	//TWIC.CTRL = TWI_SDAHOLD_50NS_gc;
	TWIC.MASTER.CTRLB = TWI_MASTER_SMEN_bm; //| TWI_MASTER_TIMEOUT1_bm;
	TWIC.MASTER.BAUD = 110;
	TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

uint8_t i2c_write_data(uint8_t slave_addr, uint8_t register_addr, uint8_t data) {
	
	TWIC.MASTER.ADDR = (slave_addr << 1);
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.DATA = register_addr;
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.DATA = data;
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	
	TWIC.MASTER.CTRLC = 0x03; //Stop
	
	return 0;
}

uint8_t i2c_read_data(uint8_t slave_addr, uint8_t register_addr) {
	
	uint8_t tmp;
	
	TWIC.MASTER.ADDR = (slave_addr << 1);
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.DATA = register_addr;
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.ADDR = (slave_addr << 1) | 0x01; //Send START
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
	tmp = TWIC.MASTER.DATA;
	TWIC.MASTER.CTRLC |= TWI_MASTER_ACKACT_bm; //Send NACK
	
	_delay_ms(1);
	
	TWIC.MASTER.CTRLC = 0x03; //Send STOP
	
	return tmp;
}

uint8_t i2c_read_data_block(uint8_t slave_addr, uint8_t register_addr, uint8_t *data, uint8_t length) {
	
	uint8_t tmp;
	uint8_t i;
	
	TWIC.MASTER.ADDR = (slave_addr << 1);
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.DATA = register_addr;
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.ADDR = (slave_addr << 1) | 0x01; //Send START
	
	for (i = 0; i < length; i++) {
		while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
		data[i] = TWIC.MASTER.DATA;
	}
	TWIC.MASTER.CTRLC |= TWI_MASTER_ACKACT_bm; //Send NACK
	//tmp = TWIC.MASTER.DATA;
	
	_delay_ms(1);
	
	TWIC.MASTER.CTRLC = 0x03; //Send STOP
	
	return length;
}

uint8_t i2c_write_data_block(uint8_t slave_addr, uint8_t register_addr, uint8_t *data, uint8_t length) {
	
	uint8_t i;
	
	TWIC.MASTER.ADDR = (slave_addr << 1);
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	TWIC.MASTER.DATA = register_addr;
	while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	
	for (i = 0; i < length; i++) {
		TWIC.MASTER.DATA = data[i];
		while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
	}
	
	TWIC.MASTER.CTRLC = 0x03; //Stop
	
	return length;
}