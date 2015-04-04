/*
 * power.c
 *
 * Created: 12/20/2014 3:27:51 PM
 *  Author: Administrator
 */ 

#include "power.h"

void lowpower_init(void) {
	DISABLE_GEN();
	DISABLE_TC();
	DISABLE_COM();
	DISABLE_ANLG();
	ENABLE_PULLUP();
}