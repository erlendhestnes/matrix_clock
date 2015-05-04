/*
 * spi.h
 *
 * Created: 12/13/2014 5:52:47 PM
 *  Author: Administrator
 */ 

#ifndef _SERCOM_H_
#define _SERCOM_H_

#include "../../global.h"

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

void spi_on(void);
void spi_off(void);
uint8_t spi_wr_rd(uint8_t spi_data);

static inline void ss_select(void) {
	SD_PORT.OUTCLR = SD_CS;
}

static inline void ss_deselect(void) {
	SD_PORT.OUTSET = SD_CS;
}

void twi_setup(TWI_t* const TWI);
void twi_stop_transmission(TWI_t* const TWI);
uint8_t twi_start_transmission(TWI_t* const TWI, const uint8_t slave_address, const uint8_t timeout_ms);
uint8_t twi_send_byte(TWI_t* const TWI, const uint8_t data);
uint8_t twi_receive_byte(TWI_t* const TWI, uint8_t* const data, const uint8_t end_of_data);
uint8_t twi_read_packet(TWI_t* const TWI, const uint8_t slave_address, const uint8_t timeout_ms, const uint8_t reg, uint8_t* data, uint8_t length);
uint8_t twi_write_packet(TWI_t* const TWI, const uint8_t slave_address, const uint8_t timeout_ms, const uint8_t reg, const uint8_t* data, uint8_t length);
#endif