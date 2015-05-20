/*
 * menu.h
 *
 * Created: 5/1/2015 10:35:50 PM
 *  Author: Administrator
 */ 


#ifndef MENU_H_
#define MENU_H_

#include "global.h"
#include "drivers/sensors/si114x/Si114x_types.h"

void menu_state_machine(SI114X_IRQ_SAMPLE *samples);

#endif /* MENU_H_ */