/*
 * dma_functions.h
 *
 * Created: 5/21/2015 4:39:12 PM
 *  Author: Administrator
 */ 


#ifndef DMA_FUNCTIONS_H_
#define DMA_FUNCTIONS_H_

bool BlockMemCopy( const void * src,
void * dest,
uint16_t blockSize,
volatile DMA_CH_t * dmaChannel );

bool MultiBlockMemCopy( const void * src, void * dest, uint16_t blockSize,
                          uint8_t repeatCount, volatile DMA_CH_t * dmaChannel );



#endif /* DMA_FUNCTIONS_H_ */