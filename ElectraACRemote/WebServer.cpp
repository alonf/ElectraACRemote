#include "WebServer.h"
#include "Util.h"
#include "WebSettings.h"
#include "ACView.h"
#include <map>
#include "Configuration.h"

using namespace std;
///*static*/ char WebServer::_setupHtmlBuffer[3072]; //for setup html result
WebServer::WebServer(WiFiManagerPtr_t wifiManager, int port, const char *appKey, unique_ptr<DeviceSettings> deviceSettings, function<void(const ACState&)> updateACState, function<ACState ()> getCurrentACState) :
	_deviceSettings(move(deviceSettings)),
	_server(port), 
	_authorizedUrl(String("/") + appKey),
	UpdateACState(updateACState),
	GetCurrentACState(getCurrentACState)
{
	_server.on("/", [this]() { HandleError(); });
	_server.on((_authorizedUrl + "/view.css").c_str(), [this]() { HandleSendViewCSS(); });
	_server.on((_authorizedUrl + "/ap_script.js").c_str(), [this]() { HandleSendAPScript(); });
	_server.on((_authorizedUrl + "/aplist.html").c_str(), [this]() { HandleSendAPList(); });
	_server.on((_authorizedUrl + "/setup").c_str(), [this]() { HandleSetup(); });
	_server.on((_authorizedUrl + "/setconfiguration").c_str(),HTTP_POST, [this]() { HandleSetConfiguration(); });
	_server.on((_authorizedUrl + "/resetaccesspoint").c_str(), [this]() { HandleResetAccessPoint(); });
	_server.on(_authorizedUrl.c_str(), [this]() { HandleMain(); });
	_server.on((_authorizedUrl + "/").c_str(), [this]() { HandleMain(); });
	_server.on((_authorizedUrl).c_str(), [this]() { HandleMain(); });
	_server.on((_authorizedUrl + "/sendacstate").c_str(), HTTP_POST, [this]() { SendACState(); });
	_server.on((_authorizedUrl + "/setacstate").c_str(), HTTP_POST, [this]() { SetACState(); });
	_server.on((_authorizedUrl + "/turnon").c_str(), HTTP_POST, [this]() { TurnOn(); });
	_server.on((_authorizedUrl + "/turnoff").c_str(), HTTP_POST, [this]() { TurnOff(); });
	_server.on((_authorizedUrl + "/getacstate").c_str(), HTTP_GET, [this]() { GetACState(); });
	_server.on((_authorizedUrl + "/getcurrenttemperature").c_str(), HTTP_GET, [this]() { HandleGetCurrentTemperature(); });
	_server.onNotFound([this]() { HandleError(); });

	wifiManager->RegisterClient([this](ConnectionStatus status) { UpdateStatus(status); });
}

void WebServer::SendBackHtml(const String &message)
{
	auto html = String("<html><body><h2>") + _header + "</h2>"
		+ message + "</body></html>";
	_server.send(200,
		"text/html",
		html.c_str());
}
void WebServer::HandleError()
{
	_server.send(401,
		"text/plain",
		"Unauthorized");
}

bool WebServer::CheckSecurity()
{
	auto apiKey = _server.arg("ApiKey");
	bool result = apiKey == ApiKey;
	if (!result)
	{
		Serial.printf("ApiKey error, ApiKey:%s instead of %s\n", apiKey.c_str(), ApiKey);
		HandleError();
	}
	return result;
}

bool WebServer::GetServerArgBoolValue(String argName)
{
	auto argText = _server.arg(argName);
	argText.toUpperCase();
	return argText.startsWith("TRUE");
}

void WebServer::SendACState()
{
	if (!CheckSecurity())
		return;
	
	ACState state;
	state.isPowerToggle = GetServerArgBoolValue("IsPowerToggle");
	state.isSwingOn = GetServerArgBoolValue("IsSwingOn");
	state.isSleepModeOn = GetServerArgBoolValue("IsSleepOn");
	state.isiFeelModeOn = GetServerArgBoolValue("IsiFeelOn");
	state.temperature = _server.arg("Temperature").toInt();
	auto acMode = _server.arg("ACMode");
	acMode.toUpperCase();
	state.mode = _ACText2ModeMap[acMode];
	
	auto acFan = _server.arg("ACFan");
	acFan.toUpperCase();
	state.fan = _ACText2FanMap[acFan];
	
	if (UpdateACState)
		UpdateACState(state);

	HandleMain();
}

bool WebServer::GetSimpleJsonEntry(const String& text, String &variable, String &value, int &lastIndex)
{
	lastIndex = -1;
	int firstIndex = text.indexOf('{');
	firstIndex++; //if not found, it is the beginning, otherwise the char after the {
	//printf("*****\n\nStarting with %s at index %d\n", text.c_str(), firstIndex);
		
	int nextCm = text.indexOf(',', firstIndex);
	int nextCb = text.indexOf('}', firstIndex);

	if (nextCm < 0) nextCm = INT32_MAX;
	if (nextCb < 0) nextCb = INT32_MAX;
	
	if (nextCm == INT32_MAX && nextCb == INT32_MAX)
		return false;
	
	lastIndex = min(nextCm, nextCb);
	String newPairString = text.substring(firstIndex, lastIndex);
	
	newPairString.trim();
	//printf("Working on %s\n", newPairString.c_str());

	int endIndex;
	if (newPairString.startsWith(R"(")"))
	{
		endIndex = newPairString.indexOf('"', 1);
		variable = newPairString.substring(1, endIndex);
		++endIndex;
	} else if (newPairString.startsWith(R"(')"))
	{
		endIndex = newPairString.indexOf('\'', 1);
		variable = newPairString.substring(1, endIndex);
		++endIndex;
	}
	else
	{
		endIndex = newPairString.indexOf(':');
		variable = newPairString.substring(1, endIndex);
	}

	//printf("Found variable name: %s\n", variable.c_str());

	firstIndex = newPairString.indexOf(':', endIndex) + 1;
	String valueString = newPairString.substring(firstIndex);
	valueString.trim();
	//printf("Working on value from: %s\n", valueString.c_str());
	if (valueString.startsWith(R"(")"))
	{
		endIndex = valueString.indexOf('"', 1);
		value = valueString.substring(1, endIndex);
	}
	else if (valueString.startsWith(R"(')"))
	{
		endIndex = valueString.indexOf('\'', 1);
		value = valueString.substring(1, endIndex);
	}
	else
	{
		value = valueString;
	}
	//printf("Found value: %s\n", value.c_str());
	++lastIndex;
	//printf("Next index: %d\n", lastIndex);
	return true;
}

void WebServer::PopulateJsonStringMap(const String& body, std::map<String, String> &result)
{
	auto text = body;
	int lastIndex;
	String variable, value;
	do
	{
		if (!GetSimpleJsonEntry(text, variable, value, lastIndex))
			break;
		result[variable] = value;
		text = text.substring(lastIndex);
	} while (text.length() > 0);
}



void WebServer::SetACState()
{
	if (!CheckSecurity())
		return;

	std::map<String, String> json;

	auto body = _server.arg("plain");
	PopulateJsonStringMap(body, json);
	Serial.printf("%s\n", body.c_str());

	ACState state;
	state.isPowerToggle = json["isPowerToggle"].equalsIgnoreCase("true");
	Serial.printf("isPowerToggle: %d\n", state.isPowerToggle);
	state.isSwingOn = json["isSwingOn"].equalsIgnoreCase("true");
	Serial.printf("isSwingOn: %d\n", state.isSwingOn);
	state.isSleepModeOn = json["isSleepOn"].equalsIgnoreCase("true"); 
	Serial.printf("isSleepModeOn: %d\n", state.isSleepModeOn);
	state.isiFeelModeOn = json["isiFeelOn"].equalsIgnoreCase("true");
	Serial.printf("isiFeelModeOn: %d\n", state.isiFeelModeOn);

	state.temperature = json["temperature"].toInt();
	Serial.printf("temperature: %d\n", state.temperature);

	auto acMode = json["mode"];
	acMode.toUpperCase();
	state.mode = _ACText2ModeMap[acMode];
	Serial.printf("acMode: %s\n", acMode.c_str());

	auto acFan = json["fan"];
	acFan.toUpperCase();
	state.fan = _ACText2FanMap[acFan];
	Serial.printf("acFan: %s\n", acFan.c_str());

	if (UpdateACState)
		UpdateACState(state);

	GetACState();
}


void WebServer::HandleGetCurrentTemperature()
{
	if (!CheckSecurity())
		return;

	auto temperature = _dhtReader.ReadTemperature();
	char tempstr[16];
	dtostrf(temperature, 2, 1, tempstr);

	String json = String(R"({ "temperature" : ")") + tempstr + R"("})";
	_server.send(200, "application/json", json);
}



void WebServer::GetACState()
{
	if (!CheckSecurity())
		return;

	if (!GetCurrentACState)
	{
		_server.send(500, "Internal Server Error");
		return;
	}

	//Get it from the registered AC manager
	auto state = GetCurrentACState();
	auto resultTemplate = R"(
	{
			"isPowerToggle" : "%s",
			"mode" : "%s",
			"fan" : "%s",
			"temperature" : %d,
			"isSwingOn" : "%s",
			"isSleepModeOn" : "%s",
			"isiFeelOn" : "%s"
	})";

	char json[256];
		
	sprintf(json, resultTemplate,
		state.isPowerToggle ? "true" : "false",
		_ACMode2TextMap[state.mode].c_str(),
		_ACFan2TextMap[state.fan].c_str(),
		state.temperature,
		state.isSwingOn ? "true" : "false",
		state.isSleepModeOn ? "true" : "false",
		state.isiFeelModeOn ? "true" : "false"
	);

	_server.send(200, "application/json", json);
}

void WebServer::SetACOnState(bool action) 
{
	if (!CheckSecurity())
		return;

	//for now we will toggle the state, but when we will have a sensor that will 
	//provide the AC working status, we will use it
	if (!GetCurrentACState)
	{
		_server.send(500, "Internal Server Error");
		return;
	}

	//Get it from the registered AC manager
	auto state = GetCurrentACState();
	state.isPowerToggle = true;
	UpdateACState(state);
	GetACState(); //response with the new state
}

void WebServer::TurnOn()
{
	SetACOnState(true);
}

void WebServer::TurnOff()
{
	SetACOnState(false);
}

void WebServer::HandleMain() 
{
	ProcessHTTPACView();
}

void WebServer::ProcessHTTPSetupRequest()
{
	PopulateHTMLSetupFromTemplate(WebSettingHtmlTemplate, _templateValuesMap);
	_server.send_P(200, "text/html", _htmlBuffer);
}

void WebServer::ProcessHTTPACView()
{
	if (!GetCurrentACState)
	{
		_server.send(500, "Internal Server Error");
		return;
	}

	//Get it from the registered AC manager
	auto state = GetCurrentACState();

	Util::StringMap valueMap;
	valueMap["ApiKey"] = ApiKey;
	valueMap["Title"] = _deviceSettings->ACNameStr;
	valueMap["ACModeAuto"] = state.mode == ACMode::Auto ? R"(checked="checked")" : "";
	valueMap["ACModeCool"] = state.mode == ACMode::Cool ? R"(checked="checked")" : "";
	valueMap["ACModeHeat"] = state.mode == ACMode::Heat ? R"(checked="checked")" : "";
	valueMap["ACModeFan"] = state.mode == ACMode::Fan ? R"(checked="checked")" : "";
	valueMap["ACModeDry"] = state.mode == ACMode::Dry ? R"(checked="checked")" : "";

	valueMap["FanModeAuto"] = state.fan == ACFan::Auto ? R"(checked="checked")" : "";
	valueMap["FanModeLow"] = state.fan == ACFan::Low ? R"(checked="checked")" : "";
	valueMap["FanModeMedium"] = state.fan == ACFan::Medium ? R"(checked="checked")" : "";
	valueMap["FanModeHigh"] = state.fan == ACFan::High ? R"(checked="checked")" : "";

	valueMap["PowerToggle"] = state.isPowerToggle ? R"(checked="checked")" : "";
	valueMap["Swing"] = state.isSwingOn ? R"(checked="checked")" : "";
	valueMap["Sleep"] = state.isSleepModeOn ? R"(checked="checked")" : "";
	valueMap["iFeel"] = state.isiFeelModeOn ? R"(checked="checked")" : "";
	valueMap["Temperature"] = String(state.temperature);

	PopulateHTMLSetupFromTemplate(ACViewHtmlTemplate, valueMap);
	_server.send_P(200, "text/html", _htmlBuffer);
}



void WebServer::HandleSendAPScript()
{
	_server.send_P(200, "text/javascript", APScript);
}

void WebServer::HandleSetup()
{
	_templateValuesMap.clear();
	_templateValuesMap["ApiKey"] = ApiKey;
	_templateValuesMap["WFPwd"] = _deviceSettings->accessPointPassword;
	_templateValuesMap["ACNameStr"] = _deviceSettings->ACNameStr;
	_templateValuesMap["PBLng"] = String(_deviceSettings->longButtonPeriod).c_str();
	_templateValuesMap["PBVLng"] = String(_deviceSettings->veryLongButtonPeriod).c_str();

	ProcessHTTPSetupRequest();
}

void WebServer::HandleSendViewCSS()
{
	_server.send_P(200, "text/css", ViewCSS);
}

void WebServer::HandleSendAPList()
{
	auto AccessPointList = ConnectionStatus::GetAccessPoints();
	String html;
	for (auto &&ap : AccessPointList)
	{

		html += R"(<input type = "radio" class="element radio" name = "ap" value = ")";
		html += ap.SSID.c_str();
		html += R"(" )";
		if (ap.SSID == _deviceSettings->ssidName)
			html += R"( checked="checked" )";
		html += R"("/> <label class="choice">)";
		html += ap.SSID.c_str();
		html += R"( <font color="purple">  Signal:)";
		html += String(100 + ap.RSSI).c_str();
		if (!ap.isEncripted)
			html += " Secure";
		html += "</font></label>";

	}
	_server.send(200, "text/html", html);
}

//extern void pp_soft_wdt_stop();
//extern void pp_soft_wdt_restart();

void WebServer::PopulateHTMLSetupFromTemplate(const String &htmlTemplate, const Util::StringMap & map) 
{
	//setup template processing variables
	int templateIndex = 0;
	int templateBufferIndex = 0;
	int templateEnd = -1;
	memset(_htmlBuffer, 0, sizeof(_htmlBuffer)); //clear the html result buffer

	int yieldTime = millis();
	do
	{
		if (millis() - yieldTime > 25) //0.025 seconds between yield
		{
			yieldTime = millis();
			yield();
		}

		Serial.printf("Continue setup template processing, index: %d\n", templateIndex);
		int beginVariable = htmlTemplate.indexOf('%', templateIndex); //search <%= by searching %
		int endVariable = -1;
		if (beginVariable >= 0) //only if beginVariable didn't reach the end of html
			endVariable = htmlTemplate.indexOf('%', beginVariable + 1);

		if (beginVariable < 0 || endVariable < 0) //no more variables
		{
			auto rest = htmlTemplate.substring(templateIndex); //add the template end
			memcpy(_htmlBuffer + templateBufferIndex, rest.c_str(), rest.length()); //copy the template tail
			break;
		}

		if (htmlTemplate[beginVariable - 1] != '<' || htmlTemplate[beginVariable + 1] != '=' || htmlTemplate[endVariable + 1] != '>') //not <%= ... %>
		{
			_htmlBuffer[templateBufferIndex++] = htmlTemplate[templateIndex];
			++templateIndex;
			continue;
		}
		auto variableName = htmlTemplate.substring(beginVariable + 2, endVariable);
		String replacedValue = map.at(variableName); //extract only the variable name and replace it
		String htmlUntilVariable = htmlTemplate.substring(templateIndex, beginVariable - 1);

		//Add all text before the variable and the replacement
		memcpy(_htmlBuffer + templateBufferIndex, htmlUntilVariable.c_str(), htmlUntilVariable.length());
		templateBufferIndex += htmlUntilVariable.length();
		memcpy(_htmlBuffer + templateBufferIndex, replacedValue.c_str(), replacedValue.length());
		templateBufferIndex += replacedValue.length();
		_htmlBuffer[templateBufferIndex] = 0;
		templateIndex = endVariable + 2;
	} while (templateIndex != templateEnd);
}

void WebServer::HandleSetConfiguration()
{
	_deviceSettings->ssidName = _server.arg("ap").c_str();
	_deviceSettings->accessPointPassword = _server.arg("WFPwd").c_str();
	_deviceSettings->ACNameStr = _server.arg("ACName").c_str();
	_deviceSettings->longButtonPeriod = atoi(_server.arg("PBLng").c_str());
	_deviceSettings->veryLongButtonPeriod = atoi(_server.arg("PBVLng").c_str());
	printf("Server arguments:\n");
	for (int i = 0; i < _server.args(); ++i)
	{
		Serial.printf("%s=%s\n", _server.argName(i).c_str(), _server.arg(_server.argName(i)).c_str());
	}

	String html =
		R"(<p><center><h3>The device will reboot and try to connect to:</h3></center></p><p>)";
		html += _deviceSettings->ssidName;
		html += "</p><br/>";
		html += "If after the reboot the two Leds are blinking or the green led is not turned on, do a factory reset by pressing the button for more than ";
		html += String(_deviceSettings->veryLongButtonPeriod / 1000).c_str();
		html += " seconds. The two leds should blink very fast.";

		SendBackHtml(html.c_str());
		_configurationUpdater(*_deviceSettings.get());
		Util::software_Reboot();
}

void WebServer::HandleResetAccessPoint()
{
	_deviceSettings->isFactoryReset = true;
	String html =
		R"(<p><h3>Access point credentials has been reset.</h3></p><br/><p>Reset device to activate access point mode.</p><br/>)";
		html += R"(<p>Set new access point SSID information by surfing to )";
		html += String(R"(http://192.168.4.1/)") + CreateUrl("setup").c_str();
		html += "</p>";

	SendBackHtml(html.c_str());
	_configurationUpdater(*_deviceSettings.get()); //this will reset the device
}

bool WebServer::IsConnected() const
{
	return WiFi.status() == WL_CONNECTED;
}

void WebServer::Loop()
{
	_server.handleClient();
}

void WebServer::SetUpdateConfiguration(std::function<void(const DeviceSettings&)> configurationUpdater)
{
	_configurationUpdater = configurationUpdater;
}

void WebServer::UpdateStatus(ConnectionStatus status)
{
	if (!_isInit && status.IsJustConnected()) //new connection, only once
	{
		Serial.println("Web Server begins...");
		_isInit = true;
		MDNS.begin("esp8266");
		_server.begin();
	}
}

String WebServer::CreateUrl(const String& s) const
{
	return _authorizedUrl + "/" + s;
}
