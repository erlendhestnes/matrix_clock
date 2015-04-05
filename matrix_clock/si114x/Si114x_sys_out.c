#include <stdio.h>
//#include <windows.h>
#include "Si114x_types.h"
//#include "Si114x_PGM_Toolkit.h"
#include "Si114x_functions.h"
#include "../sercom.h"
#include "../si114x.h"
#include <util/delay.h>

extern HANDLE mcu_handle;

void PortSet(u8 port, u8 portdata)
{
    //static u8 current_data[3] = {0xff, 0xff, 0xff};
//
    //if ( portdata != current_data[port] )
    //{
        //PT_i2c_smbus_write_byte_data(mcu_handle, port, portdata);
        //current_data[port] = portdata;
    //}
}

s16 PortRead(u8 port)
{
    //return PT_i2c_smbus_read_byte_data(mcu_handle, port) < 0 ;
}

s16 Si114xWriteToRegister(HANDLE si114x_handle, u8 address, u8 value)
{
    //return PT_i2c_smbus_write_byte_data(si114x_handle, address, value);
	return i2c_write_data(SI114X_ADDR,address,value);
}

s16 Si114xReadFromRegister(HANDLE si114x_handle, u8 address)
{
    //return PT_i2c_smbus_read_byte_data(si114x_handle, address);
	return i2c_read_data(SI114X_ADDR,address);
}

s16 Si114xBlockWrite(HANDLE si114x_handle, 
                        u8 address, u8 length, u8 *values)
{
    //return PT_i2c_smbus_write_i2c_block_data(si114x_handle,
    //                       address,    length,           values);
	return i2c_write_data_block(SI114X_ADDR,address,values,length);
}

s16 Si114xBlockRead(HANDLE si114x_handle, 
                        u8 address, u8 length, u8 *values)
{
    //return PT_i2c_smbus_read_i2c_block_data(si114x_handle,
    //                       address,    length,     values);
	return i2c_read_data_block(SI114X_ADDR,address,values,length);
}

void delay_10ms()
{
    // This is needed immediately after a reset command to the Si114x
    // In the PGM_Toolkit, there is sufficient latency, so none is added
    // here. This is a reminder that when porting code, that this must
    // be implemented.
	_delay_ms(10);
}


