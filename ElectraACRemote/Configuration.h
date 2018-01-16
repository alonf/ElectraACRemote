// Configuration.h

#ifndef _CONFIGURATION_h
#define _CONFIGURATION_h

//Wifi Configuration
static const char *SSID = "ACIoT"; //The default configuration access point ssid
static const char *password = "12345678"; //The default configuration access point password - at least 8 chars
static const char *webSiteHeader = "Welcome to Fliess Ground Level Electra AC Web Controller";
static const char *ApiKey = "your key";

//Commands
static const char *ACOnCool24 = "Turn AC On Mode:Cooling Temperature:24";
static const char *ACOffCool24 = "Turn AC Off";
static const char *FanModeFast = "Fan Mode: Fast";
static const char *FanModeMedium = "Fan Mode: Medium";
static const char *FanModeSlow = "Fan Mode: Slow";
static const char *FanModeAuto = "Fan Mode: Auto";

const unsigned int defaultButtonLongTimePeriod = 5000; //5 seconds -> reset
const unsigned int defaultButtonVeryLongTimePeriod = 20000; //20 seconds -> factory reset

//NodeMCU configuration
const int pushButton = D1; 
const int redLed = D2;
const int greenLed = D8; 
const int IRLED = D5;
const int ButtonPressed = HIGH;
const int ButtonReleased = ButtonPressed == HIGH ? LOW : HIGH;
const int DhtPin = D4;

#endif

