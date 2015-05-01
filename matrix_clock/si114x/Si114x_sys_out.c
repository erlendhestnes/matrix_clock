#include <stdio.h>
//#include <windows.h>
#include "Si114x_types.h"
//#include "Si114x_PGM_Toolkit.h"
#include "Si114x_functions.h"
#include "../sercom.h"
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
	return 1;
}

s16 Si114xWriteToRegister(HANDLE si114x_handle, u8 address, u8 val)
{
	twi_write_packet(&TWIC,SI114X_ADDR,1000,address,&val,1);
	return 0;
}

s16 Si114xReadFromRegister(HANDLE si114x_handle, u8 address)
{
	u8 val;
	twi_read_packet(&TWIC,SI114X_ADDR,1000,address,&val,1);
	return val;
}

s16 Si114xBlockWrite(HANDLE si114x_handle,
u8 address, u8 length, u8 *values)
{
	return twi_write_packet(&TWIC,SI114X_ADDR,1000,address,values,length);
}

s16 Si114xBlockRead(HANDLE si114x_handle,
u8 address, u8 length, u8 *values)
{
	return twi_read_packet(&TWIC,SI114X_ADDR,1000,address,values,length);
}

void delay_10ms()
{
	// This is needed immediately after a reset command to the Si114x
	// In the PGM_Toolkit, there is sufficient latency, so none is added
	// here. This is a reminder that when porting code, that this must
	// be implemented.
	_delay_ms(10);
}

