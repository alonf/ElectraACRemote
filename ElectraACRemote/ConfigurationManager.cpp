#include "ConfigurationManager.h"
#include <EEPROM.h>
#include "Util.h"
#include "Configuration.h"

ConfigurationManager::ConfigurationManager()
{
	EEPROM.begin(512);
	delay(10);
	if (CheckMagicNumber())
	{
		ReadEEPROMInfo();
	}
	else //factory reset
	{
		Util::String2Array(SSID, _eepromInformationBlock.SSIDName);
		Util::String2Array(password, _eepromInformationBlock.AccessPointPassword);
		_eepromInformationBlock.ACName[0] = 0;
		_eepromInformationBlock.milliSecondsButonLongTimePeriod = defaultButtonLongTimePeriod;
		_eepromInformationBlock.milliSecondsButonVeryLongTimePeriod= defaultButtonVeryLongTimePeriod;
	}
}

void ConfigurationManager::FacrotyReset()
{
	ClearMagicNumber();
	EEPROM.commit();
	ESP.restart();
}

void ConfigurationManager::FlashEEProm()
{
	FlashEEPROMInfo();
}

bool ConfigurationManager::CheckMagicNumber() const
{
	for (int i = 0; i < sizeof(_magicNumber); ++i)
		if (EEPROM.read(i) != _magicNumber[i])
			return false;
	return true;
}

void ConfigurationManager::WriteMagicNumber()
{
	for (int i = 0; i < sizeof(_magicNumber); ++i)
		EEPROM.write(i, _magicNumber[i]);
}

void ConfigurationManager::ClearMagicNumber() //used to reset password
{
	for (int i = 0; i < sizeof(_magicNumber); ++i)
		EEPROM.write(i, 0);
}

void ConfigurationManager::ReadEEPROMInfo()
{
	for (int i = 0; i < sizeof(_eepromInformationBlock); ++i)
	{
		char c = EEPROM.read(i);
		*(reinterpret_cast<char *>(&_eepromInformationBlock) + i) = c;
	}
}

void ConfigurationManager::FlashEEPROMInfo()
{
	DumpEEPromInfo();
	WriteMagicNumber();
	for (int i = sizeof(_eepromInformationBlock._magicNumber); i < sizeof(_eepromInformationBlock); ++i)
		EEPROM.write(i, *(reinterpret_cast<char *>(&_eepromInformationBlock) + i));
	EEPROM.commit();
}

void ConfigurationManager::DumpEEPromInfo()
{
	Serial.printf("EEProm Configuration Block:\n");
	Serial.printf("SSIDName: %s\n", _eepromInformationBlock.SSIDName);
	Serial.printf("AccessPointPassword: %s\n", _eepromInformationBlock.AccessPointPassword);
	Serial.printf("ACName: %s\n", _eepromInformationBlock.ACName);
	Serial.printf("milliSecondsButonLongTimePeriod: %d\n", _eepromInformationBlock.milliSecondsButonLongTimePeriod);
	Serial.printf("milliSecondsButonVeryLongTimePeriod: %d\n", _eepromInformationBlock.milliSecondsButonVeryLongTimePeriod);
}


String ConfigurationManager::GetSSID() const
{
	return String(_eepromInformationBlock.SSIDName);
}

String ConfigurationManager::GetAccessPointPassword() const
{
	return String(_eepromInformationBlock.AccessPointPassword);
}

void ConfigurationManager::SetWiFiCredentials(const String& SSID, const String& password)
{
	Util::String2Array(SSID, _eepromInformationBlock.SSIDName);
	Util::String2Array(password, _eepromInformationBlock.AccessPointPassword);
}

void ConfigurationManager::SetACName(const String& ACName)
{
	Util::String2Array(ACName, _eepromInformationBlock.ACName);
}

String ConfigurationManager::GetACName() const
{
	return String(_eepromInformationBlock.ACName);
}


void ConfigurationManager::SetButonPressTimesMilliSeconds(unsigned longPeriod, unsigned veryLongPeriod)
{
	_eepromInformationBlock.milliSecondsButonLongTimePeriod = longPeriod;
	_eepromInformationBlock.milliSecondsButonVeryLongTimePeriod = veryLongPeriod;
}


