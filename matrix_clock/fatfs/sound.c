/*------------------------------------------------------------------------/
/  Sound Streamer and RIFF-WAVE file player for Mega AVRs
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/
/* Note that the audio playback function is a heavy load for 8-bit MCUs.
/  High bitrate file in CD quality will be played as buzz sound due to
/  buffer underrun. If it is the case, the sound file should be re-sampled
/  to lower sampling frequency, 8-bit.
*/

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "sound.h"
#include "../global.h"

#define NBSIZE 32
#define FCC(c1,c2,c3,c4)	(((DWORD)c4<<24)+((DWORD)c3<<16)+(c2<<8)+c1)	/* FourCC */

static inline uint8_t read_signature_byte(uint16_t Address) {
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	__asm__ ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}

uint16_t ConvertInt16toUint12(uint16_t integer)
{
	uint16_t result;
	//if (integer == 0x8000)
	//    return (0); //minimum value
	if (integer & 0x7FFF) //if 2's complement negative
	{
		integer--;
		result = ~integer;  //result is now positive value
		result = 0x7FFF-result;
	}
	else
	result = integer + 0x7FFF;
	return(result >> 4);
}

static inline uint8_t smooth(uint8_t data, float filterVal, float smoothedVal){
	return (int)(((data * (1 - filterVal)) + (smoothedVal  *  filterVal)));
}

/*-----------------------------------------------------*/
/* Sound sampling ISR                                  */

ISR(TCC0_OVF_vect)
{
	WAVFIFO *fcb = WavFifo;	/* Pointer to FIFO controls */
	UINT ri, ct;
	BYTE *buff, l1, l2, r1, r2;
	
	static uint16_t prev = 0;
		
	if (!fcb) return;
	ct = fcb->ct; ri = fcb->ri;
	buff = fcb->buff + ri;

	switch (fcb->mode) {
		case 0:		/* Mono, 8bit */
		if (ct < 1) return;
		l1 = r2 = buff[0];
		ct -= 1; ri += 1;
		break;
		case 1:		/* Stereo, 8bit */
		if (ct < 2) return;
		l1 = buff[0]; r2 = buff[1];
		ct -= 2; ri += 2;
		break;
		case 2:		/* Mono, 16bit */
		if (ct < 2) return;
		l1 = r2 = buff[1] + 128;
		ct -= 2; ri += 2;
		break;
		default:	/* Stereo, 16bit */
		if (ct < 4) return;
		l2 = buff[0]; 
		l1 = buff[1];
		r2 = buff[2];
		r1 = buff[3];
		ct -= 4; 
		ri += 4;
	}
	fcb->ct = ct;
	fcb->ri = ri & (fcb->sz_buff - 1);
	
	DACB.CH0DATA = (1<<11)+(l1>>1);
	DACB.CH1DATA = (1<<11)-(l1>>1);
	
	//prev = data;
}

static inline void speaker_on(void) {
	PORTD.DIRSET |= PIN1_bm;
	PORTD.OUTSET |= PIN1_bm;
}

static inline void speaker_off(void) {
	PORTD.OUTCLR |= PIN1_bm;
}

/*-----------------------------------------------------*/
/* Enable sound output stream                          */

int sound_start (
	WAVFIFO* fcb,	/* Pointer to the sound FIFO control structure */
	DWORD fs		/* Sampling frequency [Hz] */
)
{
	if (fs < 8000 || fs > 44100) return 0;	/* Check fs range */

	fcb->ri = 0; fcb->wi = 0; fcb->ct = 0;	/* Flush FIFO */
	WavFifo = fcb;			/* Register FIFO control structure */
	
	PORTB.DIRSET |= PIN2_bm;
	PORTB.DIRSET |= PIN3_bm;
	DACB.CTRLA |= DAC_CH0EN_bm | DAC_CH1EN_bm;
	DACB.CTRLC |= DAC_REFSEL_INT1V_gc;
	DACB.CTRLB |= DAC_CHSEL_DUAL_gc;
	
	//From calibration rows
	DACB.CH0OFFSETCAL = 0x07;
	DACB.CH0GAINCAL = 0x1B;
	DACB.CH1GAINCAL = 0x0C;
	DACB.CH1OFFSETCAL = 0x13;

	/*
	DACB.CH0OFFSETCAL = 0xE8;
	DACB.CH0GAINCAL = 0xB6;
	DACB.CH1GAINCAL = 0x0C;
	DACB.CH1OFFSETCAL = 0x13;
	*/
	
	//DACB.CH1DATA = 255;
	
	DACB.CTRLA |= DAC_ENABLE_bm;
	
	speaker_on();
	
	TCC0.CNT = 0;
	TCC0.PER = (F_CPU / 44100 - 1);
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;
	
	return 1;
}

/*-----------------------------------------------------*/
/* Disable sound output                                */

void sound_stop (void)
{
	TCC0.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	DACB.CTRLA &= ~(DAC_ENABLE_bm);
	speaker_off();

	WavFifo = 0;		/* Unregister FIFO control structure */
}



/*-----------------------------------------------------*/
/* WAV file loader                                     */

int load_wav (
	FIL *fp,			/* Pointer to the open file object to play */
	const char *title,	/* Title (file name, etc...) */
	void *work,			/* Pointer to working buffer (must be-4 byte aligned) */
	UINT sz_work		/* Size of working buffer (must be power of 2) */
)
{
	UINT md, wi, br, tc, t, btr;
	DWORD sz, ssz, offw, szwav, wsmp, fsmp, eof;
	WAVFIFO fcb;
	BYTE k, *buff = work;
	char *p, nam[NBSIZE], art[NBSIZE];


	//xprintf(PSTR("%s\n"), title);	/* Put title */

	/* Is it a WAV file? */
	if (f_read(fp, buff, 12, &br) || br != 12) return -1;
	if (LD_DWORD(&buff[0]) != FCC('R','I','F','F')) return -1;
	if (LD_DWORD(&buff[8]) != FCC('W','A','V','E')) return -1;
	eof = LD_DWORD(&buff[4]) + 8;

	/* Analyze the RIFF-WAVE header and get properties */
	nam[0] = art[0] = 0;
	md = fsmp = wsmp = offw = szwav = 0;
	while (f_tell(fp) < eof) {
		if (f_read(fp, buff, 8, &br) || br != 8) return -1;
		sz = (LD_DWORD(&buff[4]) + 1) & ~1;
		switch (LD_DWORD(&buff[0])) {
		case FCC('f','m','t',' ') :
			if (sz > 1000 || sz < 16 || f_read(fp, buff, sz, &br) || sz != br) return -1;
			if (LD_WORD(&buff[0]) != 0x1) return -1;	/* Check if LPCM */
			if (LD_WORD(&buff[2]) == 2) {	/* Channels (1 or 2) */
				md = 1; wsmp = 2;
			} else {
				md = 0; wsmp = 1;
			}
			if (LD_WORD(&buff[14]) == 16) {	/* Resolution (8 or 16) */
				md |= 2; wsmp *= 2;
			}
			fsmp = LD_DWORD(&buff[4]);		/* Sampling rate */
			break;

		case FCC('f','a','c','t') :
			f_lseek(fp, f_tell(fp) + sz);
			break;

		case FCC('d','a','t','a') :
			offw = f_tell(fp);	/* Wave data start offset */
			szwav = sz;			/* Wave data length [byte] */
			f_lseek(fp, f_tell(fp) + sz);
			break;

		case FCC('L','I','S','T'):
			sz += f_tell(fp);
			if (f_read(fp, buff, 4, &br) || br != 4) return -1;
			if (LD_DWORD(buff) == FCC('I','N','F','O')) {	/* LIST/INFO chunk */
				while (f_tell(fp) < sz) {
					if (f_read(fp, buff, 8, &br) || br != 8) return -1;
					ssz = (LD_DWORD(&buff[4]) + 1) & ~1;
					p = 0;
					switch (LD_DWORD(buff)) {
					case FCC('I','N','A','M'):		/* INAM sub-chunk */
						p = nam; break;
					case FCC('I','A','R','T'):		/* IART sub-cnunk */
						p = art; break;
					}
					if (p && ssz <= NBSIZE) {
						if (f_read(fp, p, ssz, &br) || br != ssz) return -1;
					} else {
						if (f_lseek(fp, f_tell(fp) + ssz)) return -1;
					}
				}
			} else {
				if (f_lseek(fp, sz)) return -1;	/* Skip unknown sub-chunk type */
			}
			break;

		default :	/* Unknown chunk */
			return -1;
		}
	}
	if (!szwav || !fsmp) return -1;		/* Check if valid WAV file */
	if (f_lseek(fp, offw)) return -1;	/* Seek to top of wav data */
	tc = (UINT)(szwav / fsmp / wsmp);	/* Length (sec) */

	//xprintf(PSTR("IART=%s\nINAM=%s\n"), art, nam);
	//xprintf(PSTR("Sample=%u.%ukHz/%ubit/%S\nLength=%u:%02u\n"), (UINT)(fsmp / 1000), (UINT)(fsmp / 100) % 10, (md & 2) ? 16 : 8, (md & 1) ? PSTR("st") : PSTR("mo"), tc / 60, tc % 60);

	/* Initialize stream parameters and start sound streming */
	fcb.mode = md;
	fcb.buff = buff;
	fcb.sz_buff = sz_work;
	if (!sound_start(&fcb, fsmp)) return -1;

	k = 0; wi = 0;
	while (szwav || fcb.ct >= 4) {
		if (szwav && fcb.ct <= sz_work / 2) {	/* Refill FIFO when it gets half empty */
			btr = (szwav >= sz_work / 2) ? sz_work / 2 : szwav;
			f_read(fp, &buff[wi], btr, &br);
			if (br != btr) break;
			szwav -= br;
			wi = (wi + br) & (sz_work - 1);
			cli();
			fcb.ct += br;
			sei();
		}
		//if (uart_test()) {		/* Exit if a command arrived */
		//	k = uart_getc();
		//	break;
		//}
		//t = (f_tell(fp) - offw - fcb.ct) / fsmp / wsmp;	/* Refresh time display every 1 sec */
		//if (t != tc) {
		//	tc = t;
		//xprintf(PSTR("\rTime=%u:%02u"), tc / 60, tc % 60);
		//}
	}

	sound_stop();	/* Stop sound output */

	//xputc('\n');
	return k;	/* Terminated due to -1:error, 0:eot, >0:key code */
}

