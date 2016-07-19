/*
AD5625.h Library for writing configuration data to AD5625 DAC
Last update 6/20/2016
John Freudenthal and Sean Kirkpatrick
*/

#ifndef AD5625_h	//check for multiple inclusions
#define AD5625_h

#include "Arduino.h"
#include "i2c_t3.h"

#define NumberOfChannels 4
#define InternalReference 2.5f
#define I2CTimeout 1000
#define MaxVoltageInt 0x1000

class AD5625
{
	public:
		enum class powerMode{Unknown, Normal, GND1kOhm, GND100kOhm, HighImpedance};
		enum class outputMode{Unknown, Immediate, Synchronized};
		enum class referenceMode{Unknown, Internal, External};
		enum class commandMode{Write2Register, UpdateRegister, WriteWUpdateAll, WriteWUpdate, PowerUpDown, Reset, LDACRegister, IntReference};
		AD5625();
		AD5625(uint8_t _Address);
		AD5625(uint8_t _Address, float _VRef, referenceMode _ReferenceMode);
		~AD5625();
		bool isConnected();
		uint8_t getAddress();
		void setAddress(uint8_t address);
		bool setVoltage(uint8_t Channel, float Value);
		bool setVoltage(float Value);
		int getVoltage(uint8_t Channel);
		bool setPower(uint8_t Channel, bool Active);
		bool setPower(bool Active);
		bool getPower(uint8_t Channel);
		bool setPowerMode(powerMode ModeSetting);
		bool setPowerMode(bool OnOff);
		bool useInternalReference(bool UseInternal);
		powerMode getPowerMode();
		bool setOutputModeImmediate(bool Immediate);
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
		void SetMSBLSBForPower();
		void SendI2C();
};
#endif