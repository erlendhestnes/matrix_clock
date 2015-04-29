/*
 * spi.c
 *
 * Created: 12/13/2014 5:52:36 PM
 *  Author: Administrator
 */ 

#include "sercom.h"
#include <util/delay.h>

void spi_on(void) {
	
	SD_PORT.DIRSET = SD_CS | SD_MOSI | SD_SCK;	
	SD_PORT.OUTSET = SD_CS;	
	SD_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESCALER_DIV4_gc;
}

void spi_off(void) {
	
	SD_SPI.CTRL &= ~(SPI_ENABLE_bm);
	SD_PORT.DIRCLR = SD_CS | SD_MOSI | SD_SCK;
	SD_PORT.PIN4CTRL = PORT_OPC_PULLUP_gc;
	SD_PORT.PIN5CTRL = PORT_OPC_PULLUP_gc;
	SD_PORT.PIN6CTRL = PORT_OPC_PULLUP_gc;
	SD_PORT.PIN7CTRL = PORT_OPC_PULLUP_gc;
}

uint8_t spi_wr_rd(uint8_t spi_data) {
	
	SD_SPI.DATA = spi_data;
	while(!(SPIC.STATUS & SPI_IF_bm));
	return SD_SPI.DATA;
}

//--------------LUFA-------------------------

void twi_setup(TWI_t* const TWI) {
	//TWIC.CTRL = TWI_SDAHOLD_50NS_gc;
	TWIC.MASTER.CTRLB = TWI_MASTER_SMEN_bm; //| TWI_MASTER_TIMEOUT1_bm;
	TWIC.MASTER.BAUD = 50;
	TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

void twi_stop_transmission(TWI_t* const TWI) {
	TWI->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
}

uint8_t twi_start_transmission(TWI_t* const TWI, 
const uint8_t slave_address, 
const uint8_t timeout_ms)
{
	uint16_t timeout_remaining;

	TWI->MASTER.ADDR = slave_address;

	timeout_remaining = (timeout_ms * 100);
	while (timeout_remaining)
	{
		uint8_t status = TWI->MASTER.STATUS;

		if ((status & (TWI_MASTER_WIF_bm | TWI_MASTER_ARBLOST_bm)) == (TWI_MASTER_WIF_bm | TWI_MASTER_ARBLOST_bm))
		{
			TWI->MASTER.ADDR = slave_address;
		}
		else if ((status & (TWI_MASTER_WIF_bm | TWI_MASTER_RXACK_bm)) == (TWI_MASTER_WIF_bm | TWI_MASTER_RXACK_bm))
		{
			twi_stop_transmission(TWI);
			return TWI_ERROR_SLAVE_RESPONSE_TIMEOUT;
		}
		else if (status & (TWI_MASTER_WIF_bm | TWI_MASTER_RIF_bm))
		{
			return TWI_ERROR_NO_ERROR;
		}

		_delay_us(10);
		timeout_remaining--;
	}

	if (!(timeout_remaining)) {
		if (TWI->MASTER.STATUS & TWI_MASTER_CLKHOLD_bm) {
			twi_stop_transmission(TWI);
		}
	}

	return TWI_ERROR_BUS_CAP_TIMEOUT;
}

uint8_t twi_send_byte(TWI_t* const TWI, 
const uint8_t data)
{
	TWI->MASTER.DATA = data;

	while (!(TWI->MASTER.STATUS & TWI_MASTER_WIF_bm));

	return (TWI->MASTER.STATUS & TWI_MASTER_WIF_bm) && !(TWI->MASTER.STATUS & TWI_MASTER_RXACK_bm);
}

uint8_t twi_receive_byte(TWI_t* const TWI, 
uint8_t* const data, 
const uint8_t end_of_data)
{
	if ((TWI->MASTER.STATUS & (TWI_MASTER_BUSERR_bm | TWI_MASTER_ARBLOST_bm)) == (TWI_MASTER_BUSERR_bm | TWI_MASTER_ARBLOST_bm)) {
		return false;
	}

	while (!(TWI->MASTER.STATUS & TWI_MASTER_RIF_bm));

	*data = TWI->MASTER.DATA;

	if (end_of_data)
	TWI->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
	else
	TWI->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;

	return true;
}

uint8_t twi_read_packet(TWI_t* const TWI,
const uint8_t slave_address,
const uint8_t timeout_ms,
const uint8_t reg,
uint8_t* data,
uint8_t length)
{
	uint8_t error_code;

	if ((error_code = twi_start_transmission(TWI, (slave_address << 1) | TWI_ADDRESS_WRITE,
	timeout_ms)) == TWI_ERROR_NO_ERROR)
	{
		if (!(twi_send_byte(TWI, reg)))
		{
			error_code = TWI_ERROR_SLAVE_NAK;
			return error_code;
		}

		if ((error_code = twi_start_transmission(TWI, (slave_address << 1) | TWI_ADDRESS_READ,
		timeout_ms)) == TWI_ERROR_NO_ERROR)
		{
			while (length--)
			{
				if (!(twi_receive_byte(TWI, data++, (length == 0))))
				{
					error_code = TWI_ERROR_SLAVE_NAK;
					break;
				}
			}
		}

		twi_stop_transmission(TWI);
	}

	return error_code;
}

uint8_t twi_write_packet(TWI_t* const TWI,
const uint8_t slave_address,
const uint8_t timeout_ms,
const uint8_t reg,
const uint8_t* data,
uint8_t length)
{
	uint8_t error_code;

	if ((error_code = twi_start_transmission(TWI, (slave_address & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
	timeout_ms)) == TWI_ERROR_NO_ERROR)
	{
		if (!(twi_send_byte(TWI, reg)))
		{
			error_code = TWI_ERROR_SLAVE_NAK;
			return error_code;
		}

		while (length--)
		{
			if (!(twi_send_byte(TWI, *(data++))))
			{
				error_code = TWI_ERROR_SLAVE_NAK;
				break;
			}
		}

		twi_stop_transmission(TWI);
	}

	return error_code;
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