#include "Arduino.h"
#include "i2c_t3.h"
#define NumberOfChannels 4
#define InternalReference 2.5f
#define DefaultAddress 16
using namespace std;
enum powerMode{Unknown, Normal, GND1kOhm, GND100kOhm, HighImpedance};
enum outputMode{Unknown, Immediate, Synchronized};
enum referenceMode{Unknown, Internal, External};
class AD5625
{
	public:
		AD5625();
		~AD5625();
		bool isConnected();
		uint8_t getAddress();
		void setAddress(uint8_t address);
		bool setVoltage(int Channel, float Value);
		int getVoltage(int Channel);
		bool setPower(int Channel, bool Active);
		bool getPower(int Channel);
		bool setPowerMode(powerMode ModeSetting);
		powerMode getPowerMode();
		bool setOutputMode(outputMode ModeSetting);
		outputMode getOutputMode();
		bool setReference(referenceMode ModeSetting);
		referenceMode getReference();
		void setVRefExt(float VRef);
		float getVRefExt(); 
	private:
		uint8_t Address;
		bool Power[NumberOfChannels];
		float Voltage[NumberOfChannels];
		powerMode PowerMode;
		outputMode OutputMode;
		referenceMode ReferenceMode;
		uint8_t CommandByte;
		uint8_t MSBByte;
		uint8_t LSBByte;
		const float VRefInt = InternalReference;
		float VRefExt;
};
AD5625::AD5625()
{
	PowerMode = powerMode.Unknown;
	OutputMode = outputMode.Unknown;
	ReferenceMode = referenceMode.Unknown;
	Address = DefaultAddress;
	for (int Index = 0; Index < NumberOfChannels; Index++)
	{
		Power[Index] = true;
		Voltage[Index] = 0.0f;
	}
}
AD5625::~AD5625()
{

}
uint8_t AD5625::getAddress()
{
	return Address;
}
void AD5625::setAddress(uint8_t address)
{
	Address = address;
}
bool AD5625::isConnected()
{
	
}
bool AD5625::setVoltage(int Channel, float Value)
{
	Channel = constrain(Channel,0,NumberOfChannels);
}
int AD5625::getVoltage(int Channel)
{
	Channel = constrain(Channel,0,NumberOfChannels);
}
bool AD5625::setPower(int Channel, bool Active)
{
	Channel = constrain(Channel,0,NumberOfChannels);
}
bool AD5625::getPower(int Channel)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	return Power[Channel];
}
bool AD5625::setPowerMode(powerMode ModeSetting)
{

}
powerMode AD5625::getPowerMode()
{
	return PowerMode;
}
bool AD5625::setOutputMode(outputMode ModeSetting)
{

}
outputMode AD5625::getOutputMode()
{
	return OutputMode;
}
bool AD5625::setReference(referenceMode ModeSetting)
{

}
referenceMode AD5625::getReference()
{
	return ReferenceMode;
}
void AD5625::setVRefExt(float VRef)
{
	VRefExt = Vref;
}
float AD5625::getVRefExt()
{
	return VRefExt;
}