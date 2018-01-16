#pragma once
#include <DHT.h>
#define DHTTYPE DHT22   


class DHTReader
{
private:
	DHT _dht;
public:
	DHTReader();
	~DHTReader();
	float ReadTemperature();
	float ReadHumidity();
};

