// Copyright 2010 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Provides call-back routines that are called when nD pads are processed.
// Algorithms to calculate nD pad points should be filled in here. These
// functions are called in QS_UpdateChannels.
//
// Last Modified: 2010/04/19
//

//#include "Si114x_debug.h"
#include "Si114x_types.h"

extern u16 xdata baseline[3];

//-----------------------------------------------------------------------------
// Gesture Definitions
//-----------------------------------------------------------------------------
#define NO_GESTURE      0x00
#define RIGHT_SWIPE     0x01
#define LEFT_SWIPE      0x02
#define PAUSE           0x03

//-----------------------------------------------------------------------------
// Maximum radius and timeout defs
//-----------------------------------------------------------------------------
// Limit for the radius measurements
#define MAX_RADIUS      240 //initial value 240

// Adjustable timing parameters. 1 = ~12.8ms
#define LED_STATE_TIME  (3*13)
#define PAUSE_TIMEOUT   (50*13)
#define SWIPE_TIMEOUT   (25*13)

s16 QS_Counts_to_Distance (u16 counts, u8 led);
void SliderAlgorithm(HANDLE si114x_handle, SI114X_IRQ_SAMPLE *samples, u16 scale);
void slider_algorithm_v2(HANDLE si114x_handle, SI114X_IRQ_SAMPLE *samples, u16 scale);
void slider_algorithm_v3(HANDLE si114x_handle, SI114X_IRQ_SAMPLE *samples, u16 scale);