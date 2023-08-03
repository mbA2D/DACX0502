/**************
Arduino library for the DAC80502, DAC70502, DAC60502 DACs from Texas Instruments

***************/

#ifndef DACX0502_h
#define DACX0502_h

#include "Wire.h"

#define DACX0502_VOLTAGE_REFERENCE_V	2.5
#define DACX0502_BUF_GAIN_DEFAULT		2
#define DACX0502_REF_DIV_DEFAULT		1

#define DACX0502_MIN_I2C_ADDRESS		0x48
#define DACX0502_MAX_I2C_ADDRESS		0x51



//############# COMMAND BYTES ###############
#define DACX0502_COMMAND_NOOP		0x00
#define DACX0502_COMMAND_DEVID		0x01
#define DACX0502_COMMAND_SYNC		0x02
#define DACX0502_COMMAND_CONFIG		0x03
#define DACX0502_COMMAND_GAIN		0x04
#define DACX0502_COMMAND_TRIGGER	0x05
#define DACX0502_COMMAND_BRDCAST	0x06
#define DACX0502_COMMAND_STATUS		0x07
#define DACX0502_COMMAND_DACA		0x08
#define DACX0502_COMMAND_DACB		0x09

//############# REGISTER VALUES ##############
#define DACX0502_DEVID_DAC80502			0
#define DACX0502_DEVID_DAC70502			1
#define DACX0502_DEVID_DAC60502			2

#define DACX0502_BUFGAIN_1				0
#define DACX0502_BUFGAIN_2				1

#define DACX0502_REFDIV_1				0
#define DACX0502_REFDIV_2				1

#define DACX0502_TRIG_RESET				0b1010
#define DACX0502_TRIG_LDAC				1


typedef union _dacX0502_reg
{
	uint16_t reg_16;
	struct
	{
		uint8_t low;
		uint8_t high;
	}bytes;
}dacX0502_reg;

typedef union _dacX0502_devid_reg
{
	dacX0502_reg reg;
	struct
	{
		uint16_t res2: 12; //bit 11-0
		uint8_t devid: 3; // bit 14-12
		uint8_t res1: 1; //bit 15
	}bits;
}dacX0502_devid_reg;

typedef union _dacX0502_sync_reg
{
	dacX0502_reg reg;
	struct
	{
		uint8_t daca_sync_en: 1; //bit 0
		uint8_t dacb_sync_en: 1; //bit 1
		uint8_t res2: 6; //bit 7-2
		uint8_t daca_brdcast_en: 1; // bit 8
		uint8_t dacb_brdcast_en: 1; // bit 9
		uint8_t res1: 6; //bit 15-10
	}bits;
}dacX0502_sync_reg;

typedef union _dacX0502_conf_reg
{
	dacX0502_reg reg;
	struct
	{
		uint8_t daca_pwdwn: 1; //bit 0
		uint8_t dacb_pwdwn: 1; //bit 1
		uint8_t res1: 6; //bit 2-7
		uint8_t ref_pwdwn: 1; //bit 8
		uint8_t res2: 7; //bit 9-15
	}bits;
}dacX0502_conf_reg;

typedef union _dacX0502_gain_reg
{
	dacX0502_reg reg;
	struct
	{
		uint8_t buffa_gain: 1; //bit 0
		uint8_t buffb_gain: 1; //bit 1
		uint8_t res1: 6; //bit 2-7
		uint8_t ref_div: 1; //bit 8
		uint8_t res2: 7; //bit 9-15
	}bits;
}dacX0502_gain_reg;

typedef union _dacX0502_trig_reg
{
	dacX0502_reg reg;
	struct
	{
		uint8_t soft_reset: 4; //bit 0-3
		uint8_t ldac: 1; //bit 4
		uint16_t res: 11; //bit 5-15
	}bits;
}dacX0502_trig_reg;

typedef union _dacX0502_brdcast_reg
{
	dacX0502_reg reg;
}dacX0502_brdcast_reg;

typedef union _dacX0502_status_reg
{
	dacX0502_reg reg;
	struct
	{
		uint8_t ref_alarm: 1; //bit 0
		uint16_t res: 15; //bit 4
	}bits;
}dacX0502_status_reg;

typedef union _dacX0502_dacn_reg
{
	dacX0502_reg reg;
}dacX0502_dacn_reg;

class DACX0502
{
	public:
		DACX0502(); //turns off internal reference
	
		void init(uint8_t address, TwoWire *i2c = &Wire); //set I2C address, get devid and set the type
		void reset();
		
		void set_buf_gain_a(uint8_t gain);
		void set_buf_gain_b(uint8_t gain);
		void set_ref_div(uint8_t ref_div);
		void set_vref_value(float vref_value);//actual voltage of the reference
		
		void trigger_update();
		
		void set_dac_a(float voltage);
		void set_dac_b(float voltage);
		
		void shut_down_daca(bool status);
		void shut_down_dacb(bool status);
		void shut_down_ref(bool status);
		void shut_down_all();
		
		uint16_t get_dev_id();
		
	
	private:
		uint16_t _read_reg(uint8_t reg); //reads and returns the requested register
		void _write_register(uint8_t command_byte, dacX0502_reg reg);
		uint16_t _convert_voltage_to_dac_code(float voltage, uint8_t buf_gain);
		uint8_t _wire_request_from(uint8_t num_bytes, bool stop);

		uint8_t _addr;
		TwoWire *_i2c;
		
		float _ref_v;
		uint8_t _buf_gain_a; //1 or 2
		uint8_t _buf_gain_b; //1 or 2
		uint8_t _ref_div; // 1 or 2
		uint8_t _num_bits; //hard-coded to 16 in constructor. 14 and 12 bit versions are left-aligned and will ignore lower bits, so will still work.
		bool _using_ref_internal;
};

#endif
