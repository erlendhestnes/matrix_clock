/*
 * si114x.c
 *
 * Created: 4/3/2015 5:40:57 PM
 *  Author: Administrator
 */ 

#include "si114x.h"
#include <util/delay.h>

uint16_t si114x_write_to_register(uint8_t addr, uint8_t data) {
	return i2c_write(SI114X_ADDR, addr,data);
}

uint16_t si114x_read_from_register(uint8_t addr) {
	return i2c_read(SI114X_ADDR, addr);
}

uint16_t si114x_block_write(uint8_t address, uint8_t length, uint8_t *values) {
	uint8_t retval, counter;

	for ( counter = 0; counter < length; counter++) {
		retval = si114x_write_to_register(address+counter, values[counter]);
	}

	return retval;
}

static uint16_t _wait_until_sleep(void) {
	
	uint16_t retval;
	// This loops until the Si114x is known to be in its sleep state
	// or if an i2c error occurs
	while (1) {
		retval = si114x_read_from_register(CHIP_STAT);
		if (retval == 1) break;
		if (retval <  0) return retval;
	}
	return 0;
}

static uint16_t _send_cmd(uint8_t command) {

	uint16_t  response;
	uint16_t  retval;

	// Get the response register contents
	if ((response=si114x_read_from_register(RESPONSE))<0)
	return response;

	// Double-check the response register is consistent
	while(1)
	{
		if((retval=_wait_until_sleep()) != 0) return retval;

		if(command==0) break; // Skip if the command is NOP

		retval=si114x_read_from_register(RESPONSE);
		if(retval==response) break;
		else if(retval<0) return retval;
		else response = retval;
	}

	// Send the Command
	if ((retval = si114x_write_to_register(COMMAND, command)) != 0) {
		return retval;	
	}

	// Expect a change in the response register
	while(1) {
		if(command == 0) {
			 break; // Skip if the command is NOP
		}
		retval= si114x_read_from_register(RESPONSE);
		if (retval != response) {
			break;
		} else if (retval<0) {
		    return retval;
		}
	}
	return 0;
}

uint16_t si114x_nop(void) {
	return _send_cmd(0x00);
}

uint16_t si114x_ps_force(void) {
	return _send_cmd(0x05);
}

uint16_t si114x_als_force(void) {
	return _send_cmd(0x06);
}

uint16_t si114x_ps_als_force(void) {
	return _send_cmd(0x07);
}

uint16_t si114x_ps_als_auto (void) {
	return _send_cmd(0x0F);
}

uint16_t si114x_param_read(uint8_t address) {
	// returns Parameter[address]
	uint16_t retval;
	uint8_t cmd = 0x80 + (address & 0x1F);
	if((retval=_send_cmd(cmd))!= 0) {
		return retval;
	}
	return si114x_read_from_register(PARAM_RD);
}

//-----------------------------------------------------------------------------
// Si114xParamSet writes to the PARAM_WR and CMD register
//
uint16_t si114x_param_set(uint8_t address, uint8_t value) {
	uint16_t     retval;
	uint8_t      buffer[2];
	uint16_t     response;
	
	/*
	if((retval = _wait_until_sleep())!= 0) {
		return retval;
	}
	*/

	response = si114x_read_from_register(RESPONSE);

	buffer[0]= value;
	buffer[1]= 0xA0 + (address & 0x1F);

	retval = si114x_block_write(PARAM_WR, 2, buffer);
	
	/*
	if (retval != 0) {
		return retval;
	}

	// Wait for command to finish
	while(( retval = si114x_read_from_register(RESPONSE)) == response );

	if(retval < 0) {
		return retval;	
	} else {
		return 0;	
	}
	*/
}