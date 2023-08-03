#include <DACX0502.h>
#include "Arduino.h"

DACX0502::DACX0502()
{
	_ref_v = DACX0502_VOLTAGE_REFERENCE_V;
	_buf_gain_a = DACX0502_BUF_GAIN_DEFAULT;
	_buf_gain_b = DACX0502_BUF_GAIN_DEFAULT;
	_ref_div = DACX0502_REF_DIV_DEFAULT;
	_using_ref_internal = true;
	_num_bits = 16;
}

void DACX0502::init(uint8_t address, TwoWire *i2c = &Wire) //set I2C address, get devid and set the type, turn off ref
{
	_addr = address;
	_i2c = i2c;
	
	//check if i2c->begin has already been called.
	if (TWCR == 0)
	{
		_i2c->begin();
	}
	_i2c->setWireTimeout(3000, true);
	shut_down_ref(true);
}

void DACX0502::reset()
{
	dacX0502_trig_reg trig_reg;
	trig_reg.reg.reg_16 = 0;
	trig_reg.bits.soft_reset = DACX0502_TRIG_RESET;
	
	_write_register(DACX0502_COMMAND_TRIGGER, trig_reg.reg);
	
	_buf_gain_a = DACX0502_BUF_GAIN_DEFAULT;
	_buf_gain_b = DACX0502_BUF_GAIN_DEFAULT;
	_ref_div = DACX0502_REF_DIV_DEFAULT;
	
	if (!_using_ref_internal)
	{
		shut_down_ref(true);
	}
}

void DACX0502::set_buf_gain_a(uint8_t gain)
{
	if (gain == DACX0502_BUFGAIN_1 or gain == DACX0502_BUFGAIN_2)
	{
		dacX0502_gain_reg gain_reg;
		gain_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_GAIN);
		if (gain == DACX0502_BUFGAIN_1)
		{
			gain_reg.bits.buffa_gain = DACX0502_BUFGAIN_1;
			_buf_gain_a = 1;
		}
		else if (gain == DACX0502_BUFGAIN_2)
		{
			gain_reg.bits.buffa_gain = DACX0502_BUFGAIN_2;
			_buf_gain_a = 2;
		}
		
		_write_register(DACX0502_COMMAND_GAIN, gain_reg.reg);
	}
}

void DACX0502::set_buf_gain_b(uint8_t gain)
{
	if (gain == DACX0502_BUFGAIN_1 or gain == DACX0502_BUFGAIN_2)
	{
		dacX0502_gain_reg gain_reg;
		gain_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_GAIN);
		if (gain == DACX0502_BUFGAIN_1)
		{
			gain_reg.bits.buffb_gain = DACX0502_BUFGAIN_1;
			_buf_gain_b = 1;
		}
		else if (gain == DACX0502_BUFGAIN_2)
		{
			gain_reg.bits.buffb_gain = DACX0502_BUFGAIN_2;
			_buf_gain_b = 2;
		}
		
		_write_register(DACX0502_COMMAND_GAIN, gain_reg.reg);
	}
}

void DACX0502::set_ref_div(uint8_t ref_div)
{
	if (ref_div == 1 or ref_div == 2)
	{
		dacX0502_gain_reg gain_reg;
		gain_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_GAIN);
		if (ref_div == 1)
		{
			gain_reg.bits.ref_div = DACX0502_REFDIV_1;
			_ref_div = 1;
		}
		else if (ref_div == 2)
		{
			gain_reg.bits.ref_div = DACX0502_REFDIV_2;
			_ref_div = 2;
		}
		
		_write_register(DACX0502_COMMAND_GAIN, gain_reg.reg);
	}
}

void DACX0502::trigger_update() //for synchronous updates on the DAC outputs when not using broadcast mode
{
	dacX0502_trig_reg trig_reg;
	trig_reg.reg.reg_16 = 0;
	trig_reg.bits.ldac = 1;
	
	_write_register(DACX0502_COMMAND_TRIGGER, trig_reg.reg);
}

void DACX0502::set_vref_value(float vref_value)//actual voltage of the reference
{
	_ref_v = vref_value;
}

void DACX0502::set_dac_a(float voltage)
{
	dacX0502_dacn_reg daca_reg;
	daca_reg.reg.reg_16 = _convert_voltage_to_dac_code(voltage, _buf_gain_a);
	
	_write_register(DACX0502_COMMAND_DACA, daca_reg.reg);
}

void DACX0502::set_dac_b(float voltage)
{
	dacX0502_dacn_reg dacb_reg;
	dacb_reg.reg.reg_16 = _convert_voltage_to_dac_code(voltage, _buf_gain_b);
	
	_write_register(DACX0502_COMMAND_DACB, dacb_reg.reg);
}

uint16_t DACX0502::_convert_voltage_to_dac_code(float voltage, uint8_t buf_gain)
{
	//vout = DAC_DATA / 2^N * VREFIO / DIV * GAIN
	//DAC_DATA = VOUT * 2^N * DIV / VREFIO / GAIN

	uint32_t code = uint32_t((voltage * float(uint32_t(1)<<_num_bits) * float(_ref_div) / float(_ref_v) / float(buf_gain)) + 0.5); //+0.5 to round truncate to nearest int
	if (code > (1<<_num_bits -1))
	{
		code = (1<<_num_bits -1);
	}
	return uint16_t(code);
}

void DACX0502::shut_down_daca(bool status)
{
	dacX0502_conf_reg conf_reg;
	conf_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_CONFIG);
	conf_reg.bits.daca_pwdwn = status;
	
	_write_register(DACX0502_COMMAND_CONFIG, conf_reg.reg);
}

void DACX0502::shut_down_dacb(bool status)
{
	dacX0502_conf_reg conf_reg;
	conf_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_CONFIG);
	conf_reg.bits.dacb_pwdwn = status;
	
	_write_register(DACX0502_COMMAND_CONFIG, conf_reg.reg);
}

void DACX0502::shut_down_ref(bool status)
{
	dacX0502_conf_reg conf_reg;
	conf_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_CONFIG);
	conf_reg.bits.ref_pwdwn = status;
	
	_write_register(DACX0502_COMMAND_CONFIG, conf_reg.reg);
	_using_ref_internal = !status;
}

void DACX0502::shut_down_all()
{
	dacX0502_conf_reg conf_reg;
	conf_reg.reg.reg_16 = 0;
	conf_reg.bits.daca_pwdwn = 1;
	conf_reg.bits.dacb_pwdwn = 1;
	conf_reg.bits.ref_pwdwn = 1;
	
	_write_register(DACX0502_COMMAND_CONFIG, conf_reg.reg);
}

uint16_t DACX0502::get_dev_id()
{
	dacX0502_devid_reg devid_reg;
	devid_reg.reg.reg_16 = _read_reg(DACX0502_COMMAND_DEVID);
	return devid_reg.reg.reg_16;
}

void DACX0502::_write_register(uint8_t command_reg, dacX0502_reg reg)
{
	_i2c->beginTransmission(_addr);
	_i2c->write(command_reg);
	_i2c->write(reg.bytes.high);
	_i2c->write(reg.bytes.low);
	_i2c->endTransmission(true);
}

uint16_t DACX0502::_read_reg(uint8_t reg) //reads and returns the requested register
{
	_i2c->beginTransmission(_addr);
	_i2c->write(reg);
	_i2c->endTransmission(false); //repeated start message
	
	uint8_t byte1, byte2;
	uint8_t response_length = 2;
	uint16_t result = 0;
	_wire_request_from(response_length, true); //send stop true
	
	byte1 = _i2c->read(); //MSB first
	byte2 = _i2c->read();
	
	result = (result | byte1) << 8;
	result = (result | byte2);// << 8;
	
	return result;
}

uint8_t DACX0502::_wire_request_from(uint8_t num_bytes, bool stop)
{
	uint8_t num_bytes_read = 0;
	num_bytes_read = _i2c->requestFrom(_addr, num_bytes, stop);
	uint8_t timeout_count = 0;
	while (timeout_count < 10 && _i2c->getWireTimeoutFlag())
	{
		//if a timeout occurred, and there were less than 10 timeouts, then try again.
		_i2c->clearWireTimeoutFlag();
		num_bytes_read = _i2c->requestFrom(_addr, num_bytes, stop);
		timeout_count++;
	}
	return num_bytes_read;
}
