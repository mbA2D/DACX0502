#include <DACX0502.h>

DACX0502 dac;

void setup()
{
	Serial.begin(115200);
	Serial.println("DAC Starting");
	Serial.println("Write to both outputs");
	
	dac.init(0x48);
	dac.shut_down_ref(false);
	Serial.println("Ref ON");

	dacX0502_reg dac_reg;

	dac.set_dac_a(1.5); //sets output A to 1.5V
	Serial.println("Dac A 1.5V");

	//buffer gain must be set to 2 (default) to get voltages higher than the reference (2.5V)
	dac.set_dac_b(3.5); //sets output B to 3.5V
	Serial.println("Dac B 3.5V");
}


void loop()
{
	delay(1000);
}