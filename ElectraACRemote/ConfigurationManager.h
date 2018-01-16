// ConfigurationManager.h

#ifndef _CONFIGURATIONMANAGER_h
#define _CONFIGURATIONMANAGER_h
#include <algorithm>
#include "Singleton.h"
#include "WebServer.h"


class ConfigurationManager : public Singleton<ConfigurationManager>
{
	friend class Singleton<ConfigurationManager>;
private:
	struct
	{
		unsigned char _magicNumber[6];
		unsigned char pad[2]; //allignment
		char SSIDName[32];
		char ACName[32]; //the given name for the Air Conditioner - for example: Parent Level
		char AccessPointPassword[64];
		unsigned int milliSecondsButonLongTimePeriod;
		unsigned int milliSecondsButonVeryLongTimePeriod;
	} _eepromInformationBlock;

	const unsigned char _magicNumber[6] = { 'M', 'A', 'G', 'I', 'C', 0 };
	bool CheckMagicNumber() const;
	void WriteMagicNumber();
	void ClearMagicNumber();
	void ReadEEPROMInfo();
	void FlashEEPROMInfo();
	ConfigurationManager();

 public:
	String GetSSID() const;
	String GetAccessPointPassword() const;
	void SetWiFiCredentials(const String& SSID, const String& password);
	void SetACName(const String &ACName);
	String GetACName() const;
	void SetButonPressTimesMilliSeconds(unsigned int longPeriod, unsigned int veryLongPeriod);
	unsigned int GetLongPeriodButonPressTimesMilliSeconds() const { return _eepromInformationBlock.milliSecondsButonLongTimePeriod; }
	unsigned int GetVeryLongPeriodButonPressTimesMilliSeconds() const { return _eepromInformationBlock.milliSecondsButonVeryLongTimePeriod; }
	void FacrotyReset();
	void FlashEEProm();
	bool IsAccessPointMode() const { return !CheckMagicNumber(); }
	void DumpEEPromInfo();
};

typedef std::shared_ptr<ConfigurationManager> ConfigurationManager_t;
#endif //_CONFIGURATIONMANAGER_h

