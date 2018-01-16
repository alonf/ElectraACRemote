/*
* IRelectra
* Thanks to Barak Weiss and Chris from AnalysIR
*/
#pragma once
#ifndef IRelectra_h
#define IRelectra_h
#include <IRremoteInt.h>
#include <IRremoteESP8266.h>


enum class IRElectraMode : uint8_t
{
	Cool = 0b001,
	Heat = 0b010,
	Fan = 0b101,
	Dry = 0b100,
	Auto = 0b011
};

enum class IRElectraFan : uint8_t
{
	Low = 0b00,
	Medium = 0b01,
	High = 0b10,
	Auto = 0b11
};

enum class IRElectraSwing : uint8_t
{
	Off = 0b0,
	On = 0b1
};

enum class IRElectraSleep : uint8_t
{
	Off = 0b0,
	On = 0b1
};

enum class IRElectraIFeel : uint8_t
{
	Off = 0b0,
	On = 0b1
};

enum class IRElectraPower : uint8_t
{
	None = 0b0,
	OnOffToggle = 0b1
};

class IRelectra
{
public:
	// Ctor, remote will be used to send the raw IR data
	IRelectra(IRsend* remote);

	// Sends the specified configuration to the IR led using IRremote
	bool SendElectra(IRElectraPower power, IRElectraMode mode, IRElectraFan fan, int temperature, IRElectraSwing swing, IRElectraSleep sleep, IRElectraIFeel iFeel);

private:
	IRsend* _remote;
	uint64_t EncodeElectra(IRElectraPower power, IRElectraMode mode, IRElectraFan fan, int temperature, IRElectraSwing swing, IRElectraSleep sleep, IRElectraIFeel iFeel);
	static void AddBit(unsigned int* p, int* i, char b);
};
#endif