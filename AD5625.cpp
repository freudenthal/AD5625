/*
AD5625.cpp Library for writing configuration data to AD5625 DAC
Last update 9/3/2015
John Freudenthal and Sean Kirkpatrick
*/

#include "AD5625.h"
#define combine(high,low) ( ( (uint16_t)(high << 8) ) | (uint16_t)(low) )
#define lowbyte(value) ( (uint8_t)(value) )
#define highbyte(value) ( (uint8_t)(value>>8) )
#define VRefDefault 2.5f
#define I2CAddressDefault 0x1B

AD5625::AD5625() : AD5625::AD5625(I2CAddressDefault,VRefDefault,referenceMode::External)
{

}
AD5625::AD5625(uint8_t _Address) : AD5625::AD5625(_Address,VRefDefault,referenceMode::External)
{

}
//constructor for a specific address
AD5625::AD5625(uint8_t _Address, float _Vref, referenceMode _ReferenceMode)
{
	PowerMode = powerMode::Unknown;
	OutputMode = outputMode::Unknown;
	ReferenceMode = _ReferenceMode;
	setVRefExt(_Vref);
	Address = _Address;
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
		default:
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
bool AD5625::setVoltage(float Value)
{
	return setVoltage(NumberOfChannels, Value);
}
bool AD5625::setVoltage(uint8_t Channel, float Value)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	Value = constrain(Value,0,getVRef());
	if (Channel==4)
	{
		for (uint8_t Index=0;Index < NumberOfChannels;Index++)
		{
			Voltage[Index] = Value;
		}
	}
	else
	{
		Voltage[Channel] = Value;
	}
	Value = Value / getVRef();
	uint16_t SetValue = 0;
	SetValue = (uint16_t)(Value * (pow(2,12)-1));
	SetValue = SetValue << 4;	//four least bits are not used
	MSBByte = (uint8_t)(SetValue >> 8);
	LSBByte = (uint8_t)(SetValue);
	//Serial.println(String("S:")+String(SetValue)+String(":")+String(MSBByte)+String(":")+String(LSBByte)+String(":"));
	ResetCommandByte();
	SetCommandByteAddress(Channel);
	SetCommandByteCommand(commandMode::WriteWUpdate);
	SendI2C();
	return true;
}
int AD5625::getVoltage(uint8_t Channel)
{
	Channel = constrain(Channel,0,NumberOfChannels-1);
	return Voltage[Channel];
}
bool AD5625::setPower(bool Active)
{
	return setPower(NumberOfChannels, Active);
}
bool AD5625::setPower(uint8_t Channel, bool Active)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	if (Channel==4)
	{
		for (uint8_t Index=0; Index < NumberOfChannels; Index++)
		{
			Power[Index] = Active;
		}
	}
	else
	{
		Power[Channel] = Active;
	}
	ResetCommandByte();
	SetCommandByteCommand(commandMode::PowerUpDown);
	MSBByte = 0;
	LSBByte = 0;
	if (PowerMode == powerMode::Unknown)
	{
		PowerMode = powerMode::Normal;
	}
	switch (PowerMode)
	{
		case powerMode::GND1kOhm:
			LSBByte = LSBByte | B00010000;
			break;
		case powerMode::GND100kOhm:
			LSBByte = LSBByte | B00100000;
			break;
		case powerMode::HighImpedance:
			LSBByte = LSBByte | B00110000;
			break;
		default:
			break;
	}
	for (int ChannelIndex = 0; ChannelIndex < NumberOfChannels; ChannelIndex++)
	{
		bitWrite(LSBByte, ChannelIndex, Power[ChannelIndex]);
	}
	SendI2C();
	return true;
}
bool AD5625::getPower(uint8_t Channel)
{
	return Power[Channel];
}
bool AD5625::setPowerMode(powerMode ModeSetting)
{
	PowerMode = ModeSetting;
	return true;
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
				LSBByte = B00001111;
				break;
			case outputMode::Synchronized:
			default:
				LSBByte = 0;
				break;
		}
		SendI2C();
		OutputMode = ModeSetting;
		return true;
	} else
	{
		return false;
	}
}
outputMode AD5625::getOutputMode()
{
	return OutputMode;
}
bool AD5625::useInternalReference(bool UseInternal)
{
	bool Return = false;
	if (UseInternal)
	{
		Return = setReference(referenceMode::Internal);
	}
	else
	{
		Return = setReference(referenceMode::External);
	}
	return Return;
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
				LSBByte = B00000001;
				break;
			// case outputMode::External:
			default:
				LSBByte = 0;
				break;
		}
		SendI2C();
		ReferenceMode = ModeSetting;
		return true;
	}
	else
	{
		return false;
	}
}
referenceMode AD5625::getReference()
{
	return ReferenceMode;
}
void AD5625::setVRefExt(float VRef)
{
	VRefExt = VRef;
}
float AD5625::getVRefExt()
{
	return VRefExt;
}
void AD5625::ResetCommandByte()
{
	CommandByte = 0;
}
void AD5625::SetCommandByteAddress(uint8_t Channel)
{
	// CommandByte = DACAddress;
	CommandByte = CommandByte & B11111000;
	switch (Channel)
	{
		case 0:
			break;
		case 1:
			CommandByte = CommandByte | B00000001;
			break;
		case 2:
			CommandByte = CommandByte | B00000010;
			break;
		case 3:
			CommandByte = CommandByte | B00000011;
			break;
		case 4:
			CommandByte = CommandByte | B00000111;
			break;
		default:
			break;
	}
}
void AD5625::SetCommandByteCommand(commandMode Command)
{
	CommandByte = CommandByte & B00000111;
	switch (Command)
	{
		case commandMode::Write2Register:
			CommandByte = CommandByte | B00000000;
			break;
		case commandMode::UpdateRegister:
			CommandByte = CommandByte | B00001000;
			break;
		case commandMode::WriteWUpdateAll:
			CommandByte = CommandByte | B00010000;
			break;
		case commandMode::WriteWUpdate:
			CommandByte = CommandByte | B00011000;
			break;
		case commandMode::PowerUpDown:
			CommandByte = CommandByte | B00100000;
			break;
		case commandMode::Reset:
			CommandByte = CommandByte | B00101000;
			break;
		case commandMode::LDACRegister:
			CommandByte = CommandByte | B00110000;
			break;
		case commandMode::IntReference:
			CommandByte = CommandByte | B00111000;
			break;
		default:
			break;
	}
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