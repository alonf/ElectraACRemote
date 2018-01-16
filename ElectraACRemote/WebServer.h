// WebServer.h

#ifndef _WEBSERVER_h
#define _WEBSERVER_h

#include "Arduino.h"
#include <ESP8266WebServer.h>
#undef ARDUINO_BOARD
#define ARDUINO_BOARD "generic"
#include <ESP8266mDNS.h>
#include <String>
#include <memory>
#include "WiFiManager.h"
#include "Singleton.h"
#include "PubSub.h"
#include "ArduinoLoopManager.h"
#include "Util.h"
#include "DHTReader.h"

struct DeviceSettings
{
	bool isFactoryReset;
	String ssidName;
	String accessPointPassword;
	String ACNameStr;
	unsigned int longButtonPeriod;
	unsigned int veryLongButtonPeriod;
};

enum class ACMode
{
	Cool,
	Heat,
	Fan,
	Dry,
	Auto
};

enum class ACFan
{
	Low,
	Medium,
	High,
	Auto
};

struct ACState
{
	bool isPowerToggle;
	ACMode mode;
	ACFan fan;
	int temperature;
	bool isSwingOn;
	bool isSleepModeOn;
	int isiFeelModeOn;
} ;


typedef std::function<void(const String&, int)> WebNotificationPtr_t;
class WebServer : public Singleton<WebServer>, public IProcessor
{
	friend class Singleton<WebServer>;
private:
	char _htmlBuffer[3600]; //for setup html result
	std::unique_ptr<DeviceSettings> _deviceSettings;
	ESP8266WebServer _server;
	PubSub<WebServer, const String&, int> _pubsub;
	String _header;
	const String _authorizedUrl;
	bool _isInit = false;
	std::function<void(const DeviceSettings&)> _configurationUpdater;
	DHTReader _dhtReader;

	Util::StringMap _templateValuesMap;
	std::map<String, ACMode> _ACText2ModeMap = { {"AUTO", ACMode::Auto}, {"COOL", ACMode::Cool}, {"HEAT", ACMode::Heat}, {"FAN", ACMode::Fan}, {"DRY", ACMode::Dry} };
	std::map<String, ACFan> _ACText2FanMap = { { "AUTO", ACFan::Auto },{ "HIGH", ACFan::High },{ "LOW", ACFan::Low },{ "MEDIUM", ACFan::Medium } };
	std::map<ACMode, String> _ACMode2TextMap = { { ACMode::Auto, "auto" },{ ACMode::Cool, "cool" },{ ACMode::Heat, "heat" },{ ACMode::Fan, "fan" },{ ACMode::Dry, "dry" } };
	std::map<ACFan, String> _ACFan2TextMap = { { ACFan::Auto, "auto" },{ ACFan::High, "high" },{ ACFan::Low, "low" },{ ACFan::Medium, "medium" } };
	void SendBackHtml(const String &message);
	void UpdateStatus(ConnectionStatus status);
	std::function<ACState()> GetCurrentACState;
	String CreateUrl(const String &s) const;
	bool GetServerArgBoolValue(String argName);
	void ProcessHTTPACView();
	std::function<void(const ACState &)> UpdateACState;

	void PopulateHTMLSetupFromTemplate(const String& htmlTemplate, const Util::StringMap &map);
	WebServer(WiFiManagerPtr_t wifiManager, int port, const char *appKey, std::unique_ptr<DeviceSettings> deviceSettings, std::function<void (const ACState&)> updateACState, std::function<ACState()> getCurrentACState);
	void HandleMain();
	void ProcessHTTPSetupRequest();
	void HandleSendAPScript();
	void HandleSetup();
	void HandleSendViewCSS();
	void HandleSendAPList();
	void HandleSetConfiguration();
	void HandleResetAccessPoint();
	void HandleError();
	bool CheckSecurity();
	void SendACState();
	static bool GetSimpleJsonEntry(const String& text, String& variable, String& value, int& lastIndex);
	static void PopulateJsonStringMap(const String& body, std::map<String, String>& result);
	void SetACState();
	void HandleGetCurrentTemperature();
	void GetACState();
	void SetACOnState(bool action);
	void TurnOn();
	void TurnOff();

public:
	template<typename T>
	void SetWebSiteHeader(T header) { _header = std::forward<T>(header); }
	void Register(WebNotificationPtr_t subscriber) { _pubsub.Register(subscriber); }
	bool IsConnected() const;
	void Loop();
	void SetUpdateConfiguration(std::function<void(const DeviceSettings&)> configurationUpdater);
};

typedef std::shared_ptr<WebServer> WebServerPtr_t;
#endif

