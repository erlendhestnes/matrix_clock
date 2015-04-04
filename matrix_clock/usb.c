/*
 * usb.c
 *
 * Created: 3/25/2015 3:50:45 PM
 *  Author: Administrator
 */ 

#include "usb.h"

#include <avr/io.h>
#include <stdint.h>

void usb_setup(void) {
	USB.CTRLA = USB_ENABLE_bm;
}