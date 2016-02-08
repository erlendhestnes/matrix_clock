/*
 * spi.h
 *
 * Created: 9/29/2015 9:26:08 PM
 *  Author: Administrator
 */ 


#ifndef SPI_H_
#define SPI_H_

#include "../../global.h"

#define SD_CD		PIN3_bm
#define SD_CS		PIN4_bm
#define SD_MOSI		PIN5_bm
#define SD_MISO		PIN6_bm
#define SD_SCK		PIN7_bm

void spi_setup(void);
void spi_disable(void);
uint8_t spi_wr_rd(uint8_t spi_data);

static inline void ss_select(void) {
	PORTC.OUTCLR = SD_CS;
}

static inline void ss_deselect(void) {
	PORTC.OUTSET = SD_CS;
}

#endif /* SPI_H_ */