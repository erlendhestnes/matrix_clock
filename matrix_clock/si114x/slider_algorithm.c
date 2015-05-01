#include "slider_algorithm.h"
#include "../ht1632c.h"
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
s16 QS_Counts_to_Distance (u16 counts, u8 led)
{
    u16 code   offset_1[9]    = {29, 72, 55, 75, 95, 131, 177, 238, 300};
    u16 code   slope_1[9]     = {29, 173, 102, 237, 429, 1215, 3012, 9990, 26214};	
    u16 code   piecewise_brackets_1[9] = {19805, 16015, 9607, 6838, 3014, 1666, 578, 250, 125};	

    u16 code   offset_2[9]    = {27, 39, 51, 75, 96, 132, 177, 236, 299};
    u16 code   slope_2[9]     = {27, 39, 95, 298, 538, 1481, 3637, 11457, 31208};	
    u16 code   piecewise_brackets_2[9]= {17760, 14650, 7745, 5545, 2500, 1394, 493, 207, 102};	
		
	u16 code   offset_3[9]    = {27, 39, 51, 75, 96, 132, 177, 236, 299};
	u16 code   slope_3[9]     = {27, 39, 95, 298, 538, 1481, 3637, 11457, 31208};
	u16 code   piecewise_brackets_3[9]= {17760, 14650, 7745, 5545, 2500, 1394, 493, 207, 102};

    u8 code   maxIndex = 9;
    u8 xdata   indexLinear;
    uu32 xdata distance;

    if(led==1)      
    {
       // Perform piecewise linear approximation
       indexLinear = 0;
       for (indexLinear = 0; indexLinear < maxIndex; indexLinear++)
       {
           if (counts > piecewise_brackets_1[indexLinear])
           {
               distance.u32 = (u32)counts * (u32)slope_1[indexLinear];
               distance.u16[LSB] = offset_1[indexLinear] - distance.u16[MSB];
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
               distance.u32 = (u32)counts * (u32)slope_2[indexLinear];
               distance.u16[LSB] = offset_2[indexLinear] - distance.u16[MSB];
               break;
           }
       }
    }
	else if(led==3) 
	{
		// Perform piecewise linear approximation
		indexLinear = 0;
		for (indexLinear = 0; indexLinear < maxIndex; indexLinear++)
		{
			if (counts > piecewise_brackets_3[indexLinear])
			{
				distance.u32 = (u32)counts * (u32)slope_3[indexLinear];
				distance.u16[LSB] = offset_3[indexLinear] - distance.u16[MSB];
				break;
			}
		}
	}
    else return -1;  /* Invalid channel number */

    // Set to a max value if above a certain level.
    if (distance.u16[LSB] > 250 || indexLinear == maxIndex)
    {
        distance.u16[LSB] = 250;
    }
    return (distance.u16[LSB]);
}

void SliderAlgorithm(HANDLE si114x_handle, SI114X_IRQ_SAMPLE *samples, u16 scale)
{

    // Time stamps for gesture recognition and LED state machine
    static u16 xdata LED_flash_timeout;
    static u16 xdata Pause_gesture_timeout;
    static u16 xdata SwipeStartTime;
	
	//My implementation
	static u16 previous_led_x = 0;
	static u16 previous_led_y = 0;

    // QS_GlobalCounterOverflow assumes milliseconds. Samples->timestamp is in 100 us.
    u16 xdata QS_GlobalCounterOverflow = samples->timestamp / 10;

    // x position "bucket" endpoints to determine LED states and recognize Pause gesture.
    u16 code xbucket_Array[10] = {1, 159, 272, 385, 498, 611, 724, 837, 1100, 1102};

    // P1 LED values corresponding to the position buckets
    u8 code LED_P1_Vals[10] = {0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, 0xFF};   

    // P1 patterns for LED state machine.  Sent in reverse order, '0' bits are ON
    u8 code LED_swipe_left_pattern[8]  = 
      {0xFF, 0xFE, 0xF8, 0xE1, 0x87, 0x1F, 0x7F, 0xFF};

    u8 code LED_swipe_right_pattern[8] = 
      {0xFF, 0x7F, 0x1F, 0x87, 0xE1, 0xF8, 0xFE, 0xFF};

    // LED state machine variables
    static u8 xdata LED_flash_pattern[8];
    static u8 xdata LED_State = 0;

    // Position calculation variables
    u16 xdata r1; 
    u16 xdata r2; 
	u16 xdata r3;
    uu32 xdata x; 
	uu32 xdata y; 
    u16 xdata z; 

    // Variables to calculate current "bucket" and store previous one (LED and Pause)
    u8 xdata xbucket;
    static u8 xdata last_xbucket;

    // Swipe gesture recognition variables
    static BIT out_of_range = 1;
    BIT swipe_detect = 0;

    static u16 xdata xenter = 0;
    static u16 xdata xexit  = 0;
    static u16 xdata xlast  = 0;
    s16 xdata xdiff;
    u8 xdata swipe_speed;

    int xdata ps;

    ps = (u32) samples->ps1 - (u32)baseline[0];
    if (ps < 0) ps = 0;

    // Scale it
    ps *= scale;

    // Raw IR sensor 0 and 1 are stored in channels 4 and 5
    // ALS is stored in channel 6
    // Calculate r1 and r2 distances; store in channels 0 and 1
    r1 = QS_Counts_to_Distance ((u16)ps, 1);
	
	//printf("r1 = %d , counts = %d \r\n",r1,(u16)ps);

    ps = (u32) samples->ps2 - (u32)baseline[1];
    if (ps < 0) ps = 0;

    // Scale it
    ps *= (u32)scale;

    r2 = QS_Counts_to_Distance ((u16)ps, 1);
	
	//PS3 Sensor
	ps = (u32) samples->ps3 - (u32)baseline[2];
	if (ps < 0) ps = 0;

	// Scale it
	ps *= (u32)scale;

	r3 = QS_Counts_to_Distance ((u16)ps, 1);

    //REPLACE_0_PS1(samples->ps1); 
    //REPLACE_0_PS2(samples->ps2);
    //REPLACE_0_GEN0(baseline[0]);
    //REPLACE_0_GEN1(baseline[1]); 

   ////////////////////////////////////////////////////////////////////////////
   // Position calculation and swipe gesture detection

   // Calculate x
   //x = (r1^2 - r2^2 + d^2) / (2 * d) + offset
   x.u32 = (u32)r1 * (u32)r1;
   x.u32 = x.u32 + 33000;
   x.s32 = x.u32 - ((u32)r2 * (u32)r2);
   if (x.s32 < 0)
   {
     x.s32 = 0;
   }
   x.u32 = x.u32 / 60;

   // limit x to maximum for pad
   if (x.u16[LSB] > 1101)
   {
     x.u16[LSB] = 1101;
   }
   
   // Calculate y
   //y = (r2^2 - r3^2 + d^2) / (2 * d) + offset
   y.u32 = (u32)r2 * (u32)r2;
   y.u32 = y.u32 + 33000;
   y.s32 = y.u32 - ((u32)r3 * (u32)r3);
   if (y.s32 < 0)
   {
	   y.s32 = 0;
   }
   y.u32 = y.u32 / 60;

   // limit x to maximum for pad
   if (y.u16[LSB] > 1101)
   {
	   y.u16[LSB] = 1101;
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
            xdiff = (s16)xenter-(s16)xexit;
        }
      }
      out_of_range = 1;
      if (xexit > 550)
      {
        x.u16[LSB] = 1101;
      }
      else
      {
        x.u16[LSB] = 0;
      }
      z = MAX_RADIUS;
      Pause_gesture_timeout = QS_GlobalCounterOverflow;
   }
   else
   {
      // check for start of swipe
      if (out_of_range == 1)
      {
        xenter = x.u16[LSB];
        SwipeStartTime = QS_GlobalCounterOverflow;
      }
      out_of_range = 0;

      xlast = x.u16[LSB];

      if (r1 > r2)          // 'z' is the minimum of r1 and r2 (simple z)
      {
         z = r2;
      }
      else
      {
         z = r1;
      }
   }
   /*
   uint16_t tmp_y = 8;
   ht1632c_draw_char(previous_led_x,previous_led_y,'A',0,1);
   uint16_t tmp_x = 15-((x.u16[LSB])/73);
   ht1632c_draw_char(tmp_x,tmp_y,'A',1,1);
   ht1632c_refresh_screen();
   previous_led_x = tmp_x;
   previous_led_y = tmp_y;
   */
   //printf("z:%d\r\n",z);
   int16_t tmp_x = 15-((x.u16[LSB])/30);
   
	ht1632c_motion_print("Erlend",tmp_x);
	
   // Set raw channel data for x (channel 2) and z (channel 3)
   // REPLACE_0_PS3( x.u16[LSB] );
   // REPLACE_0_AUX( z );

   // If a swipe was detected, determine the direction (L/R), set up LED state machine
   // to indicate swipe gesture, and post the swipe event.
   
   /*
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
        if (x.u16[LSB] < xbucket_Array[xbucket])
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
      if ((u8)(QS_GlobalCounterOverflow - LED_flash_timeout) >= LED_STATE_TIME)
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
      //PortSet( 1, LED_P1_Vals[xbucket]);
	  //printf("x: %d \r\n",x.u16[LSB]);


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
       //REPLACE_0_PS3( x.u16[LSB] );
       //REPLACE_0_AUX( z ); 
	   //printf("out of range \r\n");
   }
   else
   {
       // send a "no touch"
       // It is best to display this than printf. Use AUX and PS3 channels
       //REPLACE_0_PS3( 0 );
       //REPLACE_0_AUX( 0 ); 
	    //printf("no touch \r\n");
   }
   */
}


