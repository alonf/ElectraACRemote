
#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#define RAWBUF 101

#include <IRremoteInt.h>
#include <IRremoteESP8266.h>
#include "IRelectra.h"
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <dummy.h>
#include <EEPROM.h>
#include <Arduino.h>
#include "ConfigurationManager.h"
#include "ArduinoLoopManager.h"
#include "PubSub.h"
#include "Singleton.h"
#include "Util.h"
#include "WiFiManager.h"
#include "WebServer.h"
#include "Logger.h"
#include <memory>
#include "Configuration.h"
#include "PushButtonManager.h"
#include "ACManager.h"

using namespace std;

void OnButtonPressed();
void Reset();
void ResetToAccessPointMode();
LoggerPtr_t logger;
WiFiManagerPtr_t wifiManager;
PushButtonManagerPtr_t pushButtonManager;
ACManagerPtr_t acManager;
ArduinoLoopManager_t loopManager;
ConfigurationManager_t configurationManger;
WebServerPtr_t webServer;

void UpdateACState(const ACState &acState)
{
	printf("Raw ACState: Fan:%d, Mode:%d\n", acState.fan, acState.mode);
	ACManager::State state;
	state.swing = acState.isSwingOn ? IRElectraSwing::On : IRElectraSwing::Off;
	state.power = acState.isPowerToggle ? IRElectraPower::OnOffToggle : IRElectraPower::None;
	state.sleep= acState.isSleepModeOn ? IRElectraSleep::On : IRElectraSleep::Off;
	state.iFeel = acState.isiFeelModeOn ? IRElectraIFeel::On : IRElectraIFeel::Off;
	switch (acState.fan)
	{
	case ACFan::Low: 
		state.fan = IRElectraFan::Low;
		break;
	case ACFan::Medium: 
		state.fan = IRElectraFan::Medium;
		break;
	case ACFan::High: 
		state.fan = IRElectraFan::High;
		break;
	case ACFan::Auto: 
	default: 
		state.fan = IRElectraFan::Auto;
	}
	
	switch (acState.mode)
	{
	case ACMode::Cool: 
		state.mode = IRElectraMode::Cool;
		break;
	case ACMode::Heat: 
		state.mode = IRElectraMode::Heat; 
		break;
	case ACMode::Fan: 
		state.mode = IRElectraMode::Fan; 
		break;
	case ACMode::Dry: 
		state.mode = IRElectraMode::Dry; 
		break;
	case ACMode::Auto: 
	default: 
		state.mode = IRElectraMode::Auto;
	}

	state.temperature = acState.temperature;
	acManager->SendState(state);
}

ACState GetCurrentACState()
{
	auto currentState = acManager->GetState();
	ACState state;
	state.isSwingOn = currentState.swing == IRElectraSwing::On;
	state.isPowerToggle = currentState.power == IRElectraPower::OnOffToggle;
	state.isSleepModeOn = currentState.sleep == IRElectraSleep::On;
	state.isiFeelModeOn = currentState.iFeel == IRElectraIFeel::On;
	switch (currentState.fan)
	{
	case IRElectraFan::Low:
		state.fan = ACFan::Low;
		break;
	case IRElectraFan::Medium:
		state.fan = ACFan::Medium;
		break;
	case IRElectraFan::High:
		state.fan = ACFan::High;
		break;
	case IRElectraFan::Auto:
	default:
		state.fan = ACFan::Auto;
	}

	switch (currentState.mode)
	{
	case IRElectraMode::Cool:
		state.mode = ACMode::Cool;
		break;
	case IRElectraMode::Heat:
		state.mode = ACMode::Heat;
		break;
	case IRElectraMode::Fan:
		state.mode = ACMode::Fan;
		break;
	case IRElectraMode::Dry:
		state.mode = ACMode::Dry;
		break;
	case IRElectraMode::Auto:
	default:
		state.mode = ACMode::Auto;
	}

	state.temperature = currentState.temperature;
	return state;
}

void SetupWebServer()
{
	auto deviceSettings = unique_ptr<DeviceSettings>(new DeviceSettings());
	deviceSettings->isFactoryReset = false;
	deviceSettings->ssidName = configurationManger->GetSSID();
	deviceSettings->accessPointPassword = configurationManger->GetAccessPointPassword();
	deviceSettings->ACNameStr = configurationManger->GetACName();
	deviceSettings->longButtonPeriod = configurationManger->GetLongPeriodButonPressTimesMilliSeconds();
	deviceSettings->veryLongButtonPeriod = configurationManger->GetVeryLongPeriodButonPressTimesMilliSeconds();

	webServer = WebServer::Create(wifiManager, 80, appKey, move(deviceSettings), UpdateACState, GetCurrentACState);
	webServer->SetWebSiteHeader(String(webSiteHeader));
	webServer->SetUpdateConfiguration([](const DeviceSettings& deviceSettings)
	{
		if (deviceSettings.isFactoryReset)
		{
			configurationManger->FacrotyReset();
			return; //never reach this line, the device should reset itself
		}

		configurationManger->SetWiFiCredentials(deviceSettings.ssidName, deviceSettings.accessPointPassword);
		configurationManger->SetButonPressTimesMilliSeconds(deviceSettings.longButtonPeriod, deviceSettings.veryLongButtonPeriod);
		configurationManger->SetACName(deviceSettings.ACNameStr);
		configurationManger->FlashEEProm();
	});
}


class PushButtonActions final : public IPushButtonActions
{
private:
	void OnPress() override { OnButtonPressed(); }
	int GetLongPressPeriod() override { return 5000; } //5 seconds
	void OnLongPressDetected() override { logger->OnLongButtonPressDetection(); }
	void OnLongPress() override { Reset(); }
	int GetVeryLongPressPeriod() override { return 20000; }//20 seconds
	void OnVeryLongPressDetected() override { logger->OnVeryLongButtonPressDetection(); }
	void OnVeryLongPress() override { ResetToAccessPointMode(); }
};

void setup()
{
	configurationManger = ConfigurationManager::Create();

	auto storedSSID = configurationManger->GetSSID();
	auto storedPassword = configurationManger->GetAccessPointPassword();

	logger = Logger::Create(redLed, greenLed, 115200);

	if (!configurationManger->IsAccessPointMode())
	{
		Serial.println("Try to connect to WiFi Access Point");
		Serial.print("Stored SSID is:");
		Serial.println(storedSSID.c_str());

		wifiManager = WiFiManager::Create(storedSSID, storedPassword, false);
	}
	else //Set access point mode
	{
		Serial.println("Create Access Point Mode");
		Serial.print("Stored SSID is:");
		Serial.println(storedSSID.c_str());

		wifiManager = WiFiManager::Create(SSID, password, true);
	}

	wifiManager->RegisterClient([](const ConnectionStatus &status) { logger->OnWiFiStatusChanged(status); });

	SetupWebServer();
	
	acManager = ACManager::Create();

	pushButtonManager = PushButtonManager::Create(pushButton, make_shared<PushButtonActions>());

	loopManager = ArduinoLoopManager::Create(initializer_list<processor_t>{logger, wifiManager, pushButtonManager,acManager, webServer });
	
	logger->TestLeds();
	configurationManger->DumpEEPromInfo();
}



void loop()
{
	loopManager->Loop();
}

void OnButtonPressed()
{
	acManager->OnButtonPressed();
}

void ResetToAccessPointMode()
{
	configurationManger->FacrotyReset(); //switch to access point mode and reset the device
}

void Reset()
{
	Util::software_Reboot();
}
