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
#include <stdint.h>

extern uint16_t xdata baseline[2];

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
#define MAX_RADIUS      240      

// Adjustable timing parameters. 1 = ~12.8ms
#define LED_STATE_TIME  (3*13)    
#define PAUSE_TIMEOUT   (50*13)
#define SWIPE_TIMEOUT   (45*13)

int QS_Counts_to_Distance (uint16_t counts, uint8_t led);

//-----------------------------------------------------------------------------
// QS_Counts_to_Distance
//-----------------------------------------------------------------------------
//
// Converts IR direct sensor measurement to a distance (mm)
//  This function uses a different curve for each LED. _1 at the end of the function
//   indicates it is for use with measurements using LED1. _2 is for LED2.
//
// In the IR Slider, the distance between LED to sensor is the same for each of
// the two LEDs. For this reason, it is possible to have a single counts to
// distance function. 
//
int QS_Counts_to_Distance (uint16_t counts, uint8_t led)
{
    uint16_t code   offset_1[9]    = {29, 72, 55, 75, 95, 131, 177, 238, 300};
    uint16_t code   slope_1[9]     = {29, 173, 102, 237, 429, 1215, 3012, 9990, 26214};	
    uint16_t code   piecewise_brackets_1[9] = {19805, 16015, 9607, 6838, 3014, 1666, 578, 250, 125};	

    uint16_t code   offset_2[9]    = {27, 39, 51, 75, 96, 132, 177, 236, 299};
    uint16_t code   slope_2[9]     = {27, 39, 95, 298, 538, 1481, 3637, 11457, 31208};	
    uint16_t code   piecewise_brackets_2[9]= {17760, 14650, 7745, 5545, 2500, 1394, 493, 207, 102};	

    uint8_t code   maxIndex = 9;
    uint8_t xdata   indexLinear;
    uint32_t xdata distance;

    if(led==1)      
    {
       // Perform piecewise linear approximation
       indexLinear = 0;
       for (indexLinear = 0; indexLinear < maxIndex; indexLinear++)
       {
           if (counts > piecewise_brackets_1[indexLinear])
           {
               distance.uint32_t = (uint32_t)counts * (uint32_t)slope_1[indexLinear];
               distance.uint16_t[LSB] = offset_1[indexLinear] - distance.uint16_t[MSB];
               break;
           }
       }
    }
    else if(led==2) 
    {
       // Perform piecewise linear approximation
       indexLinear = 0;
       for (indexLinear = 0; indexLinear < maxIndex; indexLinear++)
       {
           if (counts > piecewise_brackets_2[indexLinear])
           {
               distance.uint32_t = (uint32_t)counts * (uint32_t)slope_2[indexLinear];
               distance.uint16_t[LSB] = offset_2[indexLinear] - distance.uint16_t[MSB];
               break;
           }
       }
    }
    else return -1;  /* Invalid channel number */

    // Set to a max value if above a certain level.
    if (distance.uint16_t[LSB] > 250 || indexLinear == maxIndex)
    {
        distance.uint16_t[LSB] = 250;
    }
    return (distance.uint16_t[LSB]);
}

void SliderAlgorithm(HANDLE si114x_handle, SI114X_IRQ_SAMPLE *samples, uint16_t scale)
{

    // Time stamps for gesture recognition and LED state machine
    static uint16_t xdata LED_flash_timeout;
    static uint16_t xdata Pause_gesture_timeout;
    static uint16_t xdata SwipeStartTime;

    // QS_GlobalCounterOverflow assumes milliseconds. Samples->timestamp is in 100 us.
    uint16_t xdata QS_GlobalCounterOverflow = samples->timestamp / 10;

    // x position "bucket" endpoints to determine LED states and recognize Pause gesture.
    uint16_t code xbucket_Array[10] = {1, 159, 272, 385, 498, 611, 724, 837, 1100, 1102};

    // P1 LED values corresponding to the position buckets
    uint8_t code LED_P1_Vals[10] = {0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, 0xFF};   

    // P1 patterns for LED state machine.  Sent in reverse order, '0' bits are ON
    uint8_t code LED_swipe_left_pattern[8]  = 
      {0xFF, 0xFE, 0xF8, 0xE1, 0x87, 0x1F, 0x7F, 0xFF};

    uint8_t code LED_swipe_right_pattern[8] = 
      {0xFF, 0x7F, 0x1F, 0x87, 0xE1, 0xF8, 0xFE, 0xFF};

    // LED state machine variables
    static uint8_t xdata LED_flash_pattern[8];
    static uint8_t xdata LED_State = 0;

    // Position calculation variables
    uint16_t xdata r1; 
    uint16_t xdata r2; 
    uint32_t xdata x; 
    uint16_t xdata z; 

    // Variables to calculate current "bucket" and store previous one (LED and Pause)
    uint8_t xdata xbucket;
    static uint8_t xdata last_xbucket;

    // Swipe gesture recognition variables
    static BIT out_of_range = 1;
    BIT swipe_detect = 0;

    static uint16_t xdata xenter = 0;
    static uint16_t xdata xexit  = 0;
    static uint16_t xdata xlast  = 0;
    int xdata xdiff;
    uint8_t xdata swipe_speed;

    int xdata ps;

    ps = (uint32_t) samples->ps1 - (uint32_t)baseline[0];
    if (ps < 0) ps = 0;

    // Scale it
    ps *= scale;

    // Raw IR sensor 0 and 1 are stored in channels 4 and 5
    // ALS is stored in channel 6
    // Calculate r1 and r2 distances; store in channels 0 and 1
    r1 = QS_Counts_to_Distance ((uint16_t)ps, 1);

    ps = (uint32_t) samples->ps2 - (uint32_t)baseline[1];
    if (ps < 0) ps = 0;

    // Scale it
    ps *= (uint32_t)scale;

    r2 = QS_Counts_to_Distance ((uint16_t)ps, 2);

    //REPLACE_0_PS1(samples->ps1); 
    //REPLACE_0_PS2(samples->ps2);
    //REPLACE_0_GEN0(baseline[0]);
    //REPLACE_0_GEN1(baseline[1]); 

   ////////////////////////////////////////////////////////////////////////////
   // Position calculation and swipe gesture detection

   // Calculate x
   // x = (r1^2 - r2^2 + d^2) / (2 * d) + offset
   x.uint32_t = (uint32_t)r1 * (uint32_t)r1;
   x.uint32_t = x.uint32_t + 33000;
   x.int32_t = x.uint32_t - ((uint32_t)r2 * (uint32_t)r2);
   if (x.int32_t < 0)
   {
     x.int32_t = 0;
   }
   x.uint32_t = x.uint32_t / 60;

   // limit x to maximum for pad
   if (x.uint16_t[LSB] > 1101)
   {
     x.uint16_t[LSB] = 1101;
   }

   
   // Determine "z" and recognize whether "swipe" type gesture has occured
   if ((r1 > MAX_RADIUS) || (r2 > MAX_RADIUS))
   {
      // check for end of swipe
      if (out_of_range == 0)
      {
        xexit = xlast;

        // swipe must be completed in a desginated amount of time
        if (QS_GlobalCounterOverflow-SwipeStartTime < SWIPE_TIMEOUT && QS_GlobalCounterOverflow-SwipeStartTime >50)
        {
            swipe_detect = 1;   // indicate that a swipe was detected
            swipe_speed = SWIPE_TIMEOUT-(QS_GlobalCounterOverflow-SwipeStartTime);
            xdiff = (int)xenter-(int)xexit;
        }
      }
      out_of_range = 1;
      if (xexit > 550)
      {
        x.uint16_t[LSB] = 1101;
      }
      else
      {
        x.uint16_t[LSB] = 0;
      }
      z = MAX_RADIUS;
      Pause_gesture_timeout = QS_GlobalCounterOverflow;
   }
   else
   {
      // check for start of swipe
      if (out_of_range == 1)
      {
        xenter = x.uint16_t[LSB];
        SwipeStartTime = QS_GlobalCounterOverflow;
      }
      out_of_range = 0;

      xlast = x.uint16_t[LSB];

      if (r1 > r2)          // 'z' is the minimum of r1 and r2 (simple z)
      {
         z = r2;
      }
      else
      {
         z = r1;
      }
   }

   // Set raw channel data for x (channel 2) and z (channel 3)
   REPLACE_0_PS3( x.uint16_t[LSB] );
   REPLACE_0_AUX( z );

   // If a swipe was detected, determine the direction (L/R), set up LED state machine
   // to indicate swipe gesture, and post the swipe event.
   if (swipe_detect == 1)
   {
       if (xdiff > 150)             // Left Swipe
       {
            for (LED_State = 0; LED_State < 8; LED_State++)
            {
                LED_flash_pattern[LED_State] = LED_swipe_left_pattern[LED_State];
            } // LED_State should be 8 when this loop is finished

            // send LEFT_SWIPE gesture
            printf("                            %s: %d, %d\n", "LEFT_SWIPE", swipe_speed, xdiff) ;
       }
       else if (xdiff < -150)       // Right Swipe
       {
            for (LED_State = 0; LED_State < 8; LED_State++)
            {
                LED_flash_pattern[LED_State] = LED_swipe_right_pattern[LED_State];
            } // LED_State should be 8 when this loop is finished

            // send RIGHT_SWIPE gesture
            printf("                            %s: %d, %d\n", "RIGHT_SWIPE", swipe_speed, -xdiff) ;            
       }
   }
   // End of Position calculation and swipe gesture detection
   ////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////
   // LED Indicator update and PAUSE gesture recognition

   // determine which "bucket" the x position falls into
   for (xbucket = 0; xbucket < 10; xbucket++)
   {
        if (x.uint16_t[LSB] < xbucket_Array[xbucket])
        {
            break;
        }
   }

   // Reset pause gesture timer if movement detected
   if ((xbucket != last_xbucket)||(xbucket == 9)||(xbucket == 0))
   {
      Pause_gesture_timeout = QS_GlobalCounterOverflow;
   }
   last_xbucket = xbucket;  // store latest information for next comparison

   // LED State machine - Pause gesture recognition is also integrated here 
   if (LED_State != 0)  // indicate gesture pattern
   {
      if ((uint8_t)(QS_GlobalCounterOverflow - LED_flash_timeout) >= LED_STATE_TIME)
      {
         LED_flash_timeout = QS_GlobalCounterOverflow;

         // Write to Port 1 of the MCU
         PortSet( 1, LED_flash_pattern[(--LED_State)%8]); 

         Pause_gesture_timeout = QS_GlobalCounterOverflow;
      } // when all states are done, LED_State will be back to 0
   }
   else     // indicate position, test for PAUSE gesture
   {
      // Write to Port 1 of the MCU
      PortSet( 1, LED_P1_Vals[xbucket]);


      // check time stamp for pause
      if ((QS_GlobalCounterOverflow - Pause_gesture_timeout) >= PAUSE_TIMEOUT) 
      {
         LED_flash_timeout = QS_GlobalCounterOverflow;
         
         for (LED_State = 0; LED_State < 8; )
         {
            LED_flash_pattern[LED_State++] = 0xFF;
            LED_flash_pattern[LED_State++] = 0xFF;
            LED_flash_pattern[LED_State++] = LED_P1_Vals[xbucket];
            LED_flash_pattern[LED_State++] = LED_P1_Vals[xbucket];
         } // LED_State should be 8 when this loop is finished.

         LED_State = 16; // Set LED_State to 16 to repeat the pattern twice

         // Send off a PAUSE gesture to host.
         printf("                            %s: %d\n", "PAUSE", xbucket<<8) ;            
      }
   }

   // End of LED Indicator update and PAUSE gesture recognition
   ////////////////////////////////////////////////////////////////////////////

   if (!out_of_range)
   {
       // store x and z in pad 0
       // It is best to display this than printf. Use AUX and PS3 channels
       //REPLACE_0_PS3( x.uint16_t[LSB] );
       //REPLACE_0_AUX( z ); 
   }
   else
   {
       // send a "no touch"
       // It is best to display this than printf. Use AUX and PS3 channels
       //REPLACE_0_PS3( 0 );
       //REPLACE_0_AUX( 0 ); 
   }

}


