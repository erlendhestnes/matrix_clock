/*
 * twi.c
 *
 * Created: 9/29/2015 9:25:52 PM
 *  Author: Administrator
 */ 

#include "twi.h"

void twi_setup(TWI_t* const TWI) 
{
	//Disable power reduction for TWIC 
	PR.PRPC &= ~0x40;
	
	TWIC.MASTER.CTRLB  = 0;//TWI_MASTER_SMEN_bm; //| TWI_MASTER_TIMEOUT1_bm;
	TWIC.MASTER.BAUD   = 9;
	TWIC.MASTER.CTRLA  = TWI_MASTER_ENABLE_bm;
	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

void twi_off(void) 
{
	TWIC.MASTER.CTRLA = 0;
	
	//Enable power reduction for TWIC 
	PR.PRPC |= 0x40;
}

void twi_on(void) 
{
	//Disable power reduction for TWIC 
	PR.PRPC &= ~0x40;
	
	TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
}

void twi_stop_transmission(TWI_t* const TWI) 
{
	TWI->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
}

uint8_t twi_start_transmission(TWI_t* const TWI,
const uint8_t slave_address,
const uint16_t timeout_ms)
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
const uint16_t timeout_ms,
const uint8_t data)
{
	uint16_t timeout_remaining;
	
	TWI->MASTER.DATA = data;
	
	timeout_remaining = (timeout_ms * 100);
	while (!(TWI->MASTER.STATUS & TWI_MASTER_WIF_bm)&& timeout_remaining) {
		_delay_us(10);
		timeout_remaining--;
	}

	return (TWI->MASTER.STATUS & TWI_MASTER_WIF_bm) && !(TWI->MASTER.STATUS & TWI_MASTER_RXACK_bm);
}

uint8_t twi_receive_byte(TWI_t* const TWI,
const uint16_t timeout_ms,
uint8_t* const data,
const uint8_t end_of_data)
{
	uint16_t timeout_remaining;
	
	if ((TWI->MASTER.STATUS & (TWI_MASTER_BUSERR_bm | TWI_MASTER_ARBLOST_bm)) == (TWI_MASTER_BUSERR_bm | TWI_MASTER_ARBLOST_bm)) {
		return false;
	}
	timeout_remaining = (timeout_ms * 100);
	while (!(TWI->MASTER.STATUS & TWI_MASTER_RIF_bm) && timeout_remaining) {
		_delay_us(10);
		timeout_remaining--;
	}

	*data = TWI->MASTER.DATA;

	if (end_of_data)
	TWI->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
	else
	TWI->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;

	return true;
}

uint8_t twi_read_packet(TWI_t* const TWI,
const uint8_t slave_address,
const uint16_t timeout_ms,
const uint8_t reg,
uint8_t* data,
uint8_t length)
{
	uint8_t error_code;
	uint8_t len = length;

	if ((error_code = twi_start_transmission(TWI, (slave_address << 1) | TWI_ADDRESS_WRITE,
	timeout_ms)) == TWI_ERROR_NO_ERROR)
	{
		if (!(twi_send_byte(TWI, timeout_ms, reg)))
		{
			error_code = TWI_ERROR_SLAVE_NAK;
			return error_code;
		}

		if ((error_code = twi_start_transmission(TWI, (slave_address << 1) | TWI_ADDRESS_READ,
		timeout_ms)) == TWI_ERROR_NO_ERROR)
		{
			while (length--)
			{
				if (!(twi_receive_byte(TWI, timeout_ms, data++, (length == 0))))
				{
					error_code = TWI_ERROR_SLAVE_NAK;
					break;
				}
			}
		}

		twi_stop_transmission(TWI);
	}

	return len;
}

uint8_t twi_write_packet(TWI_t* const TWI,
const uint8_t slave_address,
const uint16_t timeout_ms,
const uint8_t reg,
const uint8_t* data,
uint8_t length)
{
	uint8_t error_code;

	if ((error_code = twi_start_transmission(TWI, (slave_address << 1) | TWI_ADDRESS_WRITE,
	timeout_ms)) == TWI_ERROR_NO_ERROR)
	{
		if (!(twi_send_byte(TWI, timeout_ms, reg)))
		{
			error_code = TWI_ERROR_SLAVE_NAK;
			return error_code;
		}

		while (length--)
		{
			if (!(twi_send_byte(TWI, timeout_ms, *(data++))))
			{
				error_code = TWI_ERROR_SLAVE_NAK;
				break;
			}
		}

		twi_stop_transmission(TWI);
	}

	return length;
}