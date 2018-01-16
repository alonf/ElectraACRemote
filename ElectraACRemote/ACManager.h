// ACManager.h

#ifndef _ACMANAGER_h
#define _ACMANAGER_h
#define RAWBUF 101

#include "arduino.h"
#include "Singleton.h"
#include "ArduinoLoopManager.h"
#include "Configuration.h"
#include "IRelectra.h"
#include <map>

class ACManager final : public Singleton<ACManager>, public IProcessor
{
	friend class Singleton<ACManager>;
private:
	IRsend _irsend;
	IRelectra _irElectra;
	

	virtual void Intialize(std::shared_ptr<ACManager> This) override;
	
	explicit ACManager() : _irsend(IRLED), _irElectra(&_irsend) {  }
	
	std::map<IRElectraMode, char *> _ACMode2TextMap = { { IRElectraMode::Auto, "auto" },{ IRElectraMode::Cool, "cool" },{ IRElectraMode::Heat, "heat" },{ IRElectraMode::Fan, "fan" },{ IRElectraMode::Dry, "dry" } };
	std::map<IRElectraFan, char *> _ACFan2TextMap = { { IRElectraFan::Auto, "auto" },{ IRElectraFan::High, "high" },{ IRElectraFan::Low, "low" },{ IRElectraFan::Medium, "medium" } };
	std::map<IRElectraSwing, char *> _ACSwingTextMap = { { IRElectraSwing::Off, "off" },{ IRElectraSwing::On, "on" } };
	std::map<IRElectraSleep, char *> _ACSleepTextMap = { { IRElectraSleep::Off, "off" },{ IRElectraSleep::On, "on" } };
	std::map<IRElectraPower, char *> _ACPowerTextMap = { { IRElectraPower::None, "none" },{ IRElectraPower::OnOffToggle, "on/off toggle" } };
	std::map<IRElectraIFeel, char *> _ACiFeelTextMap = { { IRElectraIFeel::Off, "off" },{ IRElectraIFeel::On, "on" } };
public:
	struct State
	{
		IRElectraFan fan = IRElectraFan::Auto;
		IRElectraMode mode = IRElectraMode::Auto;
		IRElectraSwing swing = IRElectraSwing::Off;
		IRElectraSleep sleep = IRElectraSleep::Off;
		IRElectraPower power = IRElectraPower::OnOffToggle;
		IRElectraIFeel iFeel = IRElectraIFeel::Off;
		int temperature = 24;
	};

	void OnButtonPressed();

	void SendState(State state);
	State GetState() const;
	void Loop() override {}

private:
	State _acStatus;
};


typedef std::shared_ptr<ACManager> ACManagerPtr_t;
#endif

