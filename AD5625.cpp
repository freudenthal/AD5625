#include "Arduino.h"
#include "i2c_t3.h"
#define combine(high,low) ( ( (uint16_t)(high << 8) ) | (uint16_t)(low) )
#define lowbyte(value) ( (uint8_t)(value) )
#define highbyte(value) ( (uint8_t)(value>>8) )
#define NumberOfChannels 4
#define InternalReference 2.5f
#define DefaultAddress 16
#define MaxVoltageInt = 0x1000
using namespace std;
enum class powerMode{Unknown, Normal, GND1kOhm, GND100kOhm, HighImpedance};
enum class outputMode{Unknown, Immediate, Synchronized};
enum class referenceMode{Unknown, Internal, External};
enum class commandMode{Write2Register, UpdateRegister, WriteWUpdateAll, WriteWUpdate, PowerUpDown, Reset, LDACRegister, IntReference};
class AD5625
{
	public:
		AD5625();
		~AD5625();
		bool isConnected();
		uint8_t getAddress();
		void setAddress(uint8_t address);
		bool setVoltage(uint8_t Channel, float Value);
		int getVoltage(uint8_t Channel);
		bool setPower(uint8_t Channel, bool Active);
		bool getPower(uint8_t Channel);
		bool setPowerMode(powerMode ModeSetting);
		powerMode getPowerMode();
		bool setOutputMode(outputMode ModeSetting);
		outputMode getOutputMode();
		bool setReference(referenceMode ModeSetting);
		referenceMode getReference();
		void setVRefExt(float VRef);
		float getVRefExt();
		float getVRef();
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
		void ResetCommandByte();
		void SetCommandByteAddress(uint8_t DACAddress);
		void SetCommandByteCommand(commandMode Command);
		void SendI2C();
};
AD5625::AD5625()
{
	PowerMode = powerMode::Unknown;
	OutputMode = outputMode::Unknown;
	ReferenceMode = referenceMode::Unknown;
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
float AD5625::getVRef()
{
	switch (ReferenceMode)
	{
		case referenceMode::Internal:
			return 2.0f*InternalReference;
		case referenceMode::External:
			return VRefExt;
	}
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
	int Status = 5;
	Wire.beginTransmission(Address);
    Status = Wire.endTransmission(I2C_STOP);
    if (Status == 0)
    {
    	return true;
    }
    else
    {
    	return false;
    }
}
bool AD5625::setVoltage(uint8_t Channel, float Value)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	Value = constrain(0,getVRef());
	Voltage[Channel] = Value;
	Value = Value / getVRef();
	uint16_t SetValue = (uint16_t)(Value * MaxVoltageInt);
	MSBByte = highbyte(SetValue);
	LSBByte = lowbyte(SetValue);
	ResetCommandByte();
	SetCommandByteAddress(Channel);
	SetCommandByteCommand(commandMode::WriteWUpdate);
	SendI2C();
}
int AD5625::getVoltage(uint8_t Channel)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	return Voltage[Channel];
}
bool AD5625::setPower(uint8_t Channel, bool Active)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	Power[Channel] = Active;
	if (ModeSetting != powerMode::Unknown)
	{
	ResetCommandByte();
	SetCommandByteCommand(commandMode::PowerUpDown);
	MSBByte = 0;
	LSBByte = ((((uint8_t)ModeSetting) - 1)<<4) | (uint8_t)15;
	for (int ChannelIndex = 0; ChannelIndex < NumberOfChannels; ChannelIndex++)
	{
		bitWrite(LSBByte, ChannelIndex, Power[ChannelIndex]);
	}
	SendI2C();
	}
}
bool AD5625::getPower(uint8_t Channel)
{
	return Power[Channel];
}
bool AD5625::setPowerMode(powerMode ModeSetting)
{
	PowerMode = ModeSetting;
}
powerMode AD5625::getPowerMode()
{
	return PowerMode;
}
bool AD5625::setOutputMode(outputMode ModeSetting)
{
	if (ModeSetting != outputMode::Unknown)
	{
		MSBByte = 0;
		ResetCommandByte();
		SetCommandByteCommand(commandMode::LDACRegister);
		switch (ModeSetting)
		{
			case outputMode::Immediate:
				LSBByte = (uint8_t)15;
				break;
			case outputMode::Synchronized:
			default:
				LSBByte = 0;
				break;
		}
		SendI2C();
		OutputMode = ModeSetting;
	}
}
outputMode AD5625::getOutputMode()
{
	return OutputMode;
}
bool AD5625::setReference(referenceMode ModeSetting)
{
	if (ModeSetting != referenceMode::Unknown)
	{
		MSBByte = 0;
		ResetCommandByte();
		SetCommandByteCommand(commandMode::IntReference);
		switch (ModeSetting)
		{
			case referenceMode::Internal:
				LSBByte = (uint8_t)1;
				break;
			case outputMode::External:
			default:
				LSBByte = 0;
				break;
		}
		SendI2C();
		ReferenceMode = ModeSetting;
	}
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
void AD5625::ResetCommandByte()
{
	CommandByte = 0;
}
void AD5625::SetCommandByteAddress(uint8_t DACAddress)
{
	CommandByte = DACAddress;
}
void AD5625::SetCommandByteCommand(commandMode Command)
{
	CommandByte = ( (uint8_t)Command ) << 3;
}
void AD5625::SendI2C()
{
  bool MoveOn = false;
  const int MaxAttempts = 16;
  int CurrentAttempt = 0;
  int SendSuccess = 5;
  while (!MoveOn)
  {
	Wire.beginTransmission(Address);
	Wire.write(CommandByte);
	Wire.write(MSBByte);
	Wire.write(LSBByte);
	SendSuccess = Wire.endTransmission(I2C_STOP,I2CTimeout);
    if(SendSuccess != 0)
    {
      Wire.finish();
      Wire.resetBus();
      CurrentAttempt++;
      if (CurrentAttempt > MaxAttempts)
      {
        MoveOn = true;
        Serial.println("Unrecoverable I2C transmission error with AD5625.");
      }
    }
    else
    {
      MoveOn = true;
    }
  }
}