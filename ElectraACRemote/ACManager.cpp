#include "ACManager.h"

using namespace std;

void ACManager::Intialize(shared_ptr<ACManager> This) //called by the singleton create, after ctor
{
	_irsend.begin();
	Serial.println("AC Manager begins...");
};


void ACManager::SendState(State state)
{
	_acStatus = state;
	if (_acStatus.temperature < 15)
		_acStatus.temperature = 15;
	if (_acStatus.temperature > 30)
		_acStatus.temperature = 30;

	_irElectra.SendElectra(_acStatus.power, _acStatus.mode, _acStatus.fan, _acStatus.temperature, _acStatus.swing, _acStatus.sleep, _acStatus.iFeel);

	Serial.printf("Send AC command with these values: Power is %s, Mode is %s, Fan is %s, Temperature is %d, Swing is %s, Sleep mode is %s iFeel mode is %s\n",
		_ACPowerTextMap[_acStatus.power],
		_ACMode2TextMap[_acStatus.mode],
		_ACFan2TextMap[_acStatus.fan],
		_acStatus.temperature,
		_ACSwingTextMap[_acStatus.swing],
		_ACSleepTextMap[_acStatus.sleep],
		_ACiFeelTextMap[_acStatus.iFeel]);
}


ACManager::State ACManager::GetState() const
{
	return _acStatus;
}

void ACManager::OnButtonPressed()
{
	SendState(State()); //default values are what we need
}




