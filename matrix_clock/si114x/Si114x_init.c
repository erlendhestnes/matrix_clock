/*
#include "Si114x_functions.h"

#define GENERAL 1

s16 si114x_init(HANDLE si114x_handle)
{ 
    s16 retval   = 0;

    u8  code current_LED1  = 0x0f;   // 359 mA
    u8  code current_LED2  = 0x0f;   // 359 mA
    u8  code current_LED3  = 0x00;   //   0 mA

    u8  tasklist      = 0x23;   // IR, PS1, PS2

    u8  measrate      = 0x84;   // 0xa0 every 30.0 ms
                                // 0x94 every 20.0 ms
                                // 0x84 every 10.4 ms
                                // 0x74 every  5.2 ms
                                // 0x70 every  4.2 ms
                                // 0x60 every  3.2 ms
    u8  psrate        = 0x08;
    u8  alsrate       = 0x08;
#ifdef GENERAL                          
    u8  code psrange  =  1;     // PS Range 
#endif

#ifdef INDOORS
    u8  code psrange  =  0;     // PS Range 
#endif

    u8  code psgain   =  0;     // PS ADC Gain

    u8  code irrange  =  1;     // IR Range
    u8  code irgain   =  0;     // IR ADC Gain

    u8  code visrange =  1;     // VIS Range
    u8  code visgain  =  0;     // VIS ADC Gain

    SI114X_CAL_S xdata si114x_cal;

    // Choose IR PD Size
    u8  code ps1pdsize     = 1;      // PD Choice for PS1
                                // 0 = Small, 1= Large
    u8  code ps2pdsize     = 1;      // PD Choice for PS2
                                // 0 = Small, 1= Large
    u8  code ps3pdsize     = 1;      // PD Choice for PS3
                                // 0 = Small, 1= Large
    u8  code irpd          = 1;      // PD Choice for IR Ambient
                                // 0 = Small, 1= Large      

    // Select which PD is enabled per PS measurement
    u8  code ps1ledsel     = LED1_EN;  
    u8  code ps2ledsel     = LED2_EN;
    u8  code ps3ledsel     = LED3_EN;


    // Turn off RTC
    retval+=Si114xWriteToRegister(si114x_handle, REG_MEAS_RATE,     0 );
    retval+=Si114xWriteToRegister(si114x_handle, REG_PS_RATE,       0 );
    retval+=Si114xWriteToRegister(si114x_handle, REG_ALS_RATE,      0 );


    // Note that the Si114xReset() actually performs the following functions:
    //     1. Pauses all prior measurements
    //     2. Clear  i2c registers that need to be cleared
    //     3. Clears irq status to make sure INT* is negated
    //     4. Delays 10 ms
    //     5. Sends HW Key
    retval+=Si114xReset(si114x_handle);

    // Program LED Currents
    {
        u8 i21, i3; 

        i21 = (current_LED2<<4) + current_LED1;
        i3  = current_LED3;
        retval+=Si114xWriteToRegister(si114x_handle, REG_PS_LED21, i21);
        retval+=Si114xWriteToRegister(si114x_handle, REG_PS_LED3 , i3);
    }

    // Initialize CHLIST Parameter from caller to enable measurement
    // Valid Tasks are: ALS_VIS_TASK, ALS_IR_TASK, PS1_TASK
    //                  PS2_TASK, PS3_TASK and AUX_TASK
    // However, if we are passed a 'negative' task, we will
    // turn on ALS_IR, ALS_VIS and PS1. Otherwise, we will use the 
    // task list specified by the caller.
    retval+=Si114xParamSet(si114x_handle, PARAM_CH_LIST, tasklist);
    
    // Set IRQ Modes and INT CFG to interrupt on every sample
    retval+=Si114xWriteToRegister(si114x_handle, REG_INT_CFG, ICG_INTOE);  
    retval+=Si114xWriteToRegister(si114x_handle, REG_IRQ_ENABLE, 
          IE_ALS_EVRYSAMPLE + 
          IE_PS1_EVRYSAMPLE + 
          IE_PS2_EVRYSAMPLE + 
          IE_PS3_EVRYSAMPLE );  

    retval+=Si114xWriteToRegister(si114x_handle, REG_IRQ_MODE1, 
          IM1_ALS_EVRYSAMPLE + 
          IM1_PS1_EVRYSAMPLE + 
          IM1_PS2_EVRYSAMPLE );

    retval+=Si114xWriteToRegister(si114x_handle, REG_IRQ_MODE2, 
          IM2_PS3_EVRYSAMPLE);
  
    retval+=Si114xParamSet(si114x_handle, PARAM_PS1_ADC_MUX, 0x03*ps1pdsize);
    retval+=Si114xParamSet(si114x_handle, PARAM_PS2_ADC_MUX, 0x03*ps2pdsize);
    retval+=Si114xParamSet(si114x_handle, PARAM_PS3_ADC_MUX, 0x03*ps3pdsize);

    retval+=Si114xParamSet(si114x_handle, PARAM_IR_ADC_MUX,  0x03*irpd);

    retval+=Si114xParamSet(si114x_handle, PARAM_PS_ADC_GAIN, psgain);
    retval+=Si114xParamSet(si114x_handle, PARAM_ALSIR_ADC_GAIN, irgain);
    retval+=Si114xParamSet(si114x_handle, PARAM_ALSVIS_ADC_GAIN, visgain);
    
    retval+=Si114xParamSet(si114x_handle, PARAM_PSLED12_SELECT, (ps2ledsel<<4)+ps1ledsel);
    retval+=Si114xParamSet(si114x_handle, PARAM_PSLED3_SELECT,  ps3ledsel);
    
    retval+=Si114xParamSet(si114x_handle, PARAM_PS_ADC_COUNTER,    RECCNT_511);
    retval+=Si114xParamSet(si114x_handle, PARAM_ALSIR_ADC_COUNTER, RECCNT_511);
    retval+=Si114xParamSet(si114x_handle, PARAM_ALSVIS_ADC_COUNTER, RECCNT_511);

    retval+=Si114xParamSet(si114x_handle, PARAM_PS_ADC_MISC,     RANGE_EN*psrange + PS_MEAS_MODE);
    retval+=Si114xParamSet(si114x_handle, PARAM_ALSIR_ADC_MISC, RANGE_EN*irrange );
    retval+=Si114xParamSet(si114x_handle, PARAM_ALSVIS_ADC_MISC,RANGE_EN*visrange);


    if( measrate > 0 )
    {
        // Set up how often the device wakes up to make measurements
        // measrate, for example can be the following values:
        //    0xa0 = Device Wakes up every ~30 ms
        //    0x94 = Device Wakes up every ~20 ms
        //    0x84 = Device Wakes up every ~10 ms
        //    0xB9 = Device Wakes up every ~100 ms
        //    0xFF = Device Wakes up every ~2 sec
        retval+=Si114xWriteToRegister(si114x_handle, REG_MEAS_RATE, measrate);

        // if 0x08, PS1, PS2 and PS3 made every time device wakes up.
        retval+=Si114xWriteToRegister(si114x_handle, REG_PS_RATE,   psrate);

        // if 0x08, VIS, IR, AUX Measurements every time device wakes up.
        retval+=Si114xWriteToRegister(si114x_handle, REG_ALS_RATE,  alsrate);

        // Enable Autonomous Operation
        retval+=Si114xPsAlsAuto(si114x_handle);
    
        // If nothing went wrong after all of this time, the value
        // returned will be 0. Otherwise, it will be some negative
        // number
    }
    return retval;
}

*/