/*
 * twi.h
 *
 * Created: 9/29/2015 9:25:59 PM
 *  Author: Administrator
 */ 


#ifndef TWI_H_
#define TWI_H_

#include "../../global.h"

enum TWI_ErrorCodes_t
{
	TWI_ERROR_NO_ERROR					= 1, /**< Indicates that the command completed successfully. */
	TWI_ERROR_BUS_FAULT					= 2, /**< A TWI bus fault occurred while attempting to capture the bus. */
	TWI_ERROR_BUS_CAP_TIMEOUT			= 3, /**< A timeout occurred whilst waiting for the bus to be ready. */
	TWI_ERROR_SLAVE_RESPONSE_TIMEOUT	= 4, /**< No ACK received at the nominated slave address within the timeout period. */
	TWI_ERROR_SLAVE_NOT_READY			= 5, /**< Slave NAKed the TWI bus START condition. */
	TWI_ERROR_SLAVE_NAK					= 6, /**< Slave NAKed whilst attempting to send data to the device. */
};

#define TWI_ADDRESS_READ         0x01
#define TWI_ADDRESS_WRITE        0x00
#define TWI_DEVICE_ADDRESS_MASK  0xFE

void twi_setup(TWI_t* const TWI);
void twi_off(void);
void twi_on(void);

void twi_stop_transmission(TWI_t* const TWI);
uint8_t twi_start_transmission(TWI_t* const TWI, const uint8_t slave_address, const uint8_t timeout_ms);
uint8_t twi_send_byte(TWI_t* const TWI, const uint8_t timeout_ms, const uint8_t data);
uint8_t twi_receive_byte(TWI_t* const TWI, const uint8_t timeout_ms, uint8_t* const data, const uint8_t end_of_data);
uint8_t twi_read_packet(TWI_t* const TWI, const uint8_t slave_address, const uint8_t timeout_ms, const uint8_t reg, uint8_t* data, uint8_t length);
uint8_t twi_write_packet(TWI_t* const TWI, const uint8_t slave_address, const uint8_t timeout_ms, const uint8_t reg, const uint8_t* data, uint8_t length);

#endif /* TWI_H_ */