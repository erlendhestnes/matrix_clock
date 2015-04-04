/*
 * si114x.h
 *
 * Created: 4/3/2015 5:41:08 PM
 *  Author: Administrator
 */ 

#ifndef _SI114X_H_
#define _SI114X_H_

#include <stdint.h>

#define SI114X_ADDR		0x5A
#define GLOBAL_ADDR		0x00
#define GLOBAL_RST		0x06

#define PART_ID			0x00
#define REV_ID			0x01
#define SEQ_ID			0x02
#define INT_CFG			0x03
#define IRQ_ENABLE		0x04
#define IRQ_MODE1		0x05
#define IRQ_MODE2		0x06
#define HW_KEY			0x07
#define MEAS_RATE		0x08
#define ALS_RATE		0x09
#define PS_RATE			0x0A
#define ALS_LOW_TH0		0x0B
#define ALS_LOW_TH1		0x0C
#define ALS_HI_TH0		0x0D
#define ALS_HI_TH1		0x0E
#define PS_LED21		0x0F
#define PS_LED3			0x10
#define PS1_TH0			0x11
#define PS1_TH1			0x12
#define PS2_TH0			0x13
#define PS2_TH1			0x14
#define PS3_TH0			0x15
#define PS3_TH1			0x16
#define PARAM_WR		0x17
#define COMMAND			0x18
#define RESPONSE		0x20
#define IRQ_STATUS		0x21
#define ALS_VIS_DATA0	0x22
#define ALS_VIS_DATA1	0x23
#define ALS_IR_DATA0	0x24
#define ALS_IR_DATA1	0x25
#define PS1_DATA0		0x26
#define PS1_DATA1		0x27
#define PS2_DATA0		0x28
#define PS2_DATA1		0x29
#define PS3_DATA0		0x2A
#define PS3_DATA1		0x2B
#define AUX_DATA0		0x2C
#define AUX_DATA1		0x2D
#define PARAM_RD		0x2E
#define CHIP_STAT		0x30
#define ANA_IN_KEY		0x3B

// Parameter Offsets
#define PARAM_I2C_ADDR            0x00
#define PARAM_CH_LIST             0x01
#define PARAM_PSLED12_SELECT      0x02
#define PARAM_PSLED3_SELECT       0x03
#define PARAM_FILTER_EN           0x04
#define PARAM_PS_ENCODING         0x05
#define PARAM_ALS_ENCODING        0x06
#define PARAM_PS1_ADC_MUX         0x07
#define PARAM_PS2_ADC_MUX         0x08
#define PARAM_PS3_ADC_MUX         0x09
#define PARAM_PS_ADC_COUNTER      0x0A
#define PARAM_PS_ADC_CLKDIV       0x0B
#define PARAM_PS_ADC_GAIN         0x0B
#define PARAM_PS_ADC_MISC         0x0C
#define PARAM_VIS_ADC_MUX         0x0D
#define PARAM_IR_ADC_MUX          0x0E
#define PARAM_AUX_ADC_MUX         0x0F
#define PARAM_ALSVIS_ADC_COUNTER  0x10
#define PARAM_ALSVIS_ADC_CLKDIV   0x11
#define PARAM_ALSVIS_ADC_GAIN     0x11
#define PARAM_ALSVIS_ADC_MISC     0x12
#define PARAM_ALS_HYST            0x16
#define PARAM_PS_HYST             0x17
#define PARAM_PS_HISTORY          0x18
#define PARAM_ALS_HISTORY         0x19
#define PARAM_ADC_OFFSET          0x1A
#define PARAM_SLEEP_CTRL          0x1B
#define PARAM_LED_RECOVERY        0x1C
#define PARAM_ALSIR_ADC_COUNTER   0x1D
#define PARAM_ALSIR_ADC_CLKDIV    0x1E
#define PARAM_ALSIR_ADC_GAIN      0x1E
#define PARAM_ALSIR_ADC_MISC      0x1F

uint16_t si114x_write_to_register(uint8_t addr, uint8_t data);
uint16_t si114x_read_from_register(uint8_t addr);
uint16_t si114x_block_write(uint8_t address, uint8_t length, uint8_t *values);
uint16_t si114x_nop(void);
uint16_t si114x_ps_force(void);
uint16_t si114x_als_force(void);
uint16_t si114x_ps_als_force(void);
uint16_t si114x_ps_als_auto (void);
uint16_t si114x_param_read(uint8_t address);
uint16_t si114x_param_set(uint8_t address, uint8_t value);

#endif