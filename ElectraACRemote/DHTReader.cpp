#include "DHTReader.h"
#include "configuration.h"

DHTReader::DHTReader() : _dht(DhtPin, DHTTYPE)
{
	_dht.begin();
}


DHTReader::~DHTReader()
{
}

float DHTReader::ReadTemperature() 
{
	for (int i = 0; i < 5; ++i) //five times attempt to read
	{
		float t = _dht.readTemperature();
		if (isnan(t))
		{
			Serial.printf("Failed to read temperature\n");
			delay(250);
			continue;
		}
		Serial.printf("Temperature is %d\n", static_cast<int>(t));
		return t;
	}
	return 24; //default in case of an error
}

float DHTReader::ReadHumidity()
{
	for (int i = 0; i < 5; ++i) //five times attempt to read
	{
		float h = _dht.readHumidity();
		if (isnan(h))
		{
			delay(250);
			continue;
		}
		return h;
	}
	return 50; //default in case of an error
}
