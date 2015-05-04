#include "Si114x_handler.h"
#include "User_defs.h"

#include <stdio.h>

u8 xdata initial_baseline_counter=128;
u16 xdata maxLeakage[3] = { 0, 0, 0};
u16 xdata baseline[3];    // Array to store calcBaseline return values

#ifdef GENERAL
u16 code noise_margin    = 10;
u16 code scale           =  8;
u8  code ircorrection[3] =  { 11, 33, 33 };
#endif

#ifdef INDOORS
u16 code noise_margin    = 50;
u16 code scale           =  3;
u8  code ircorrection[3] =  { 17, 35, 29 };
#endif

void si114x_process_samples(HANDLE si114x_handle, SI114X_IRQ_SAMPLE *samples)
{
    if ((maxLeakage[0]==0)&&(maxLeakage[1]==0)&&(maxLeakage[2]==0)&&(initial_baseline_counter==128))
    {
        printf("Computing Baseline. Make sure nothing is in the vicinity of the EVB\n");
        PortSet(1,0); // Turn on all lights to indicate baseline is being computed
    }

    if (initial_baseline_counter != 0)
    {
        // The first 128 samples are used to establish the initial 
        // baseline
        if ( initial_baseline_counter > 1 )
        {
            // Look for maximum
            if( maxLeakage[0] < samples->ps1) maxLeakage[0] = samples->ps1;
            if( maxLeakage[1] < samples->ps2) maxLeakage[1] = samples->ps2;
			if( maxLeakage[2] < samples->ps3) maxLeakage[2] = samples->ps3;
        }
        else
        {
            printf("Initial Baseline Calculated, PS1 = %d, PS2 = %d, PS3 = %d \n", maxLeakage[0], maxLeakage[1],maxLeakage[2]);
            PortSet(1,0xff); // Turn off all leds to indicate baseline computation is completed
            // Set Max Leakage 
            maxLeakage[0] = maxLeakage[0] + noise_margin*2;
            maxLeakage[1] = maxLeakage[1] + noise_margin*2;
			maxLeakage[2] = maxLeakage[2] + noise_margin*2;

            baseline[0] = maxLeakage[0];
            baseline[1] = maxLeakage[1];
			baseline[2] = maxLeakage[2];
        }
        
        // Decrement counter
        initial_baseline_counter--;
    }
    else
    {
        //
        // Once the baseline has been set, do the actual loop
        // The first thing that happens is to check for saturation events
        if(samples->ps1 > 48000 || samples->ps2 > 48000 || samples->ps3 > 48000)
        {
            //
            // Handle Saturation Events by discarding them. These 
            // are noted on the console it is also possible to 
            // handle saturation events by changing ADC settings.
            //
            // For readability of this code, no changes to 
            // the settings are done.
            //
            printf("Samples are Saturated\n");
        }
        else
        { 
            if( isIRStable( samples ) )
            {
                IRCompensation( 0, samples, ircorrection );  // IR Ambient Compensation for PS1 Channel
                IRCompensation( 1, samples, ircorrection );  // IR Ambient Compensation for PS2 Channel
				IRCompensation( 2, samples, ircorrection );  // IR Ambient Compensation for PS3 Channel

                calcBaseline( 0 , samples, noise_margin); // Calculate PS1 Baseline
                calcBaseline( 1 , samples, noise_margin); // Calculate PS2 Baseline
				calcBaseline( 2 , samples, noise_margin); // Calculate PS3 Baseline

                SliderAlgorithm(si114x_handle, samples, scale);

            }
        }
    }
}

char isIRStable( SI114X_IRQ_SAMPLE *samples)
{
    static u32 xdata IRlast = 0;              // Contains the ALS IR reading from 
                                        //     the previous measurement cycle
                                        //
    u32 xdata IR = (u32) samples->ir; // Make a copy of ALS IR Reading
    
    // Initialize IRlast to the ALS IR reading if it does not contain 
    // information from a previous measurement cycle
    if( IRlast == 0) IRlast = samples->ir; 

    // If ALS IR is saturated, return false immediately
    else if( samples->ir > 48000 ) return 0; 

    if( samples->ir < (IRlast + IRlast/2) && samples->ir > (IRlast - IRlast/2))
    {
        // If IR is within 3 dB, declare it stable
        // and perform some averaging
        IRlast = (samples->ir + (IRlast*3)) / 4;
        return 1;
    }
    else
    {
        // Otherwise, return false and update
        // new IRlast value for next time
        IRlast = samples->ir;
        return 0;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function: IRCompensation
//
// This function will correct each IR proximity channel based on ALS IR
//
// Inputs:
//    proxChannel - indicates to function which proxChannel to compensate
//          0 - PS1 (using LED1)
//          1 - PS2 (using LED2)
//          2 - PS3 (using LED3)
//    samples - raw samples
//    ircorrection - IR Correction Factors:
//          Lg PD, PSRANGE = 0 
//             use { 17, 35, 29 }
//          Lg PD, PSRANGE = 1 
//             use { 11, 33, 33 }
//          Sm PD, PSRANGE = 1 
//             use { 23, 64, 56 }
////////////////////////////////////////////////////////////////////////////////
void IRCompensation(u8 proxChannel, SI114X_IRQ_SAMPLE *samples, u8 ircorrection[])
{
    u32 xdata Correction;             // IR Correction factor

    // Calculate correction factor based on the ALS IR + PS reading 
    // subtracting out the 256 count offset contained in both measurements
    // Full equation comes out to:
    // Correction = ( (ALS_IR - 256) + (PS - 256) ) * IRcorrectionfactor
    //  where IRcorrectionfactor = IRCORRECTION / 10000    
    switch(proxChannel)
    {
        case 0:
            Correction = ((u32)samples->ir + (u32)(samples->ps1) - 512) * (u32) ircorrection[proxChannel];
            Correction = Correction/10000;
            samples->ps1 = samples->ps1 + Correction;    
            break;
        case 1:
            Correction = ((u32)samples->ir + (u32)(samples->ps2) - 512) * (u32) ircorrection[proxChannel];
            Correction = Correction/10000;
            samples->ps2 = samples->ps2 + Correction;            
            break;
		case 2:
			Correction = ((u32)samples->ir + (u32)(samples->ps3) - 512) * (u32) ircorrection[proxChannel];
			Correction = Correction/10000;
			samples->ps3 = samples->ps3 + Correction;
			break;
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Function: calcBaseline
//
// This function will calculate the baseline for a specific channel
//
// Inputs:
//    proxChannel - indicates to function which proximity channel to baseline
//          0 - PS1 (using LED1)
//          1 - PS2 (using LED2)
//          2 - PS3 (using LED3)
//          0xff - resets static vars
//    samples - raw samples 
//    noise_margin - number of ADC counts to add to baseline to avoid noise
//          50  = Lg PD, PSRANGE=0, 180 mA 
//          10  = Lg PD, PSRANGE=1, 360 mA 
//          4  = Sm PD, PSRANGE=1, 360 mA 
////////////////////////////////////////////////////////////////////////////////
void calcBaseline(u8 proxChannel, SI114X_IRQ_SAMPLE *samples, u16 noise_margin)
{
    static u16 xdata rollingMax[3] = {0,0,0}; // Stores a rolling maximum value for 
                                    // each PS channel. This will determine the 
                                    // baseline level.

    static u8 xdata Maxcount[3]= {0,0,0};     // Stores the number of measurement cycles 
                                    // that have occurred since the last time 
                                    // the rollingMax was hit by PS readings.

    u32 xdata average=0;                    // Temporary variable used to calculate 
                                    // the exponential average of PS channel

    static u16 xdata dynamic_baseline[3] = {0,0,0};

    u16 xdata *pPS;

    switch(proxChannel)
    {
        case 0:
            pPS = &samples->ps1;
            break;
        case 1:
            pPS = &samples->ps2;
            break;
        case 2:
            pPS = &samples->ps3;
            break;
        default:
            // Invalid Prox Channel. Reset all static variables 
            // to initial values
            average    = 0;
            dynamic_baseline[0] = 0;
            dynamic_baseline[1] = 0;
			dynamic_baseline[2] = 0;
            rollingMax[0] = 0;
            rollingMax[1] = 0;
			rollingMax[2] = 0;
            Maxcount[0]   = 0;
            Maxcount[1]   = 0;
			Maxcount[2]   = 0;
            return;
            break;
    }

    // If the stored baseline in dynaBaseline is still set to its initialized 
    // value of maxLeakage, then set dynaBaseline equal to the newest sample
    if ( dynamic_baseline[proxChannel] == 0 && *pPS < maxLeakage[proxChannel] )
    {
       dynamic_baseline[proxChannel] = *pPS;
    } else if ( dynamic_baseline[proxChannel] == 0)
    {
       dynamic_baseline[proxChannel] = maxLeakage[proxChannel];
    } 

    // Only process samples which are below the maxLeakage values, anything 
    // higher is a signal created by an object in front of the system which 
    // should not be baselined
    if ( *pPS < maxLeakage[proxChannel] )
    {
        average = dynamic_baseline[proxChannel]*7;
        average = average + (u32)(*pPS);
        average = average/8;

        // If the new sample has not changed the baseline average by a large 
        // amount, then allow the rollingMax variable to update. If the new 
        // sample did move the baseline average by a lot, then it's possible 
        // it was created by a proximity event. So, do not consider this part 
        // of the rollingMax
        if ( dynamic_baseline[proxChannel] <= (u16)average + (u16)average/64
            && dynamic_baseline[proxChannel] >= (u16)average - (u16)average/64)
        {
            if ( rollingMax[proxChannel] == 0 )
            {
                rollingMax[proxChannel] = *pPS;
                Maxcount[proxChannel] = 0;
            } 
            else if ( rollingMax[proxChannel] >= *pPS )
            {
                (Maxcount[proxChannel]) += 1;
                if ( Maxcount[proxChannel] > 64 )
                {
                    (rollingMax[proxChannel]) -= 1;
                    Maxcount[proxChannel] = 0;
                }
            } 
            else
            {
                rollingMax[proxChannel] = *pPS;
                Maxcount[proxChannel] = 0;
            }
        }

        // Update the stored baseline average with the new value
        dynamic_baseline[proxChannel] = (u16)average;
    }

    // Use the rollingMax value to determine the baseline level
    if ( rollingMax[proxChannel] != 0 )
    {
        baseline[proxChannel] = rollingMax[proxChannel] + noise_margin;
    } 
    else
    {  // If the rollingMax has not been determined yet, use the average
        baseline[proxChannel] = (u16) dynamic_baseline[proxChannel] + noise_margin;
    }
}


