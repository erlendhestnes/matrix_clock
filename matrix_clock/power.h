/*
 * power.h
 *
 * Created: 12/20/2014 3:28:00 PM
 *  Author: Administrator
 */ 

#include <avr/io.h>

#define __PORT_PULLUP(port, mask) { \
	PORTCFG.MPCMASK = mask ; \
	port.PIN0CTRL = PORT_OPC_PULLUP_gc; \
}

#define DISABLE_GEN( ) { \
	PR.PRGEN |= PR_AES_bm | PR_DMA_bm | PR_EVSYS_bm | PR_RTC_bm; \
}

#define DISABLE_TC( ) { \
	PR.PRPC |= PR_HIRES_bm | PR_TC0_bm | PR_TC1_bm; \
	PR.PRPD |= PR_HIRES_bm | PR_TC0_bm | PR_TC1_bm; \
	PR.PRPE |= PR_HIRES_bm | PR_TC0_bm; \
}

#define DISABLE_COM( ) { \
	PR.PRPC |= PR_SPI_bm | PR_TWI_bm | PR_USART0_bm | PR_USART1_bm; \
	PR.PRPD |= PR_SPI_bm | PR_USART0_bm | PR_USART1_bm; \
	PR.PRPE |= PR_TWI_bm | PR_USART0_bm; \
}

#define DISABLE_ANLG( ) { \
	PR.PRPA |= PR_AC_bm | PR_ADC_bm; \
	PR.PRPB |= PR_DAC_bm; \
}

#define ENABLE_PULLUP( ) { \
	__PORT_PULLUP(PORTA, 0xFF); \
	__PORT_PULLUP(PORTB, 0x0F); \
	__PORT_PULLUP(PORTC, 0xFF); \
	__PORT_PULLUP(PORTD, 0xFF); \
	__PORT_PULLUP(PORTE, 0x0F); \
	__PORT_PULLUP(PORTR, 0x03); \
}