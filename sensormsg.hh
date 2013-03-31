#ifndef SENSORMSG_HH
#define SENSORMSG_HH

#include <string>

enum SensorType
{
	DEVICE,
	TEMPERATURE,
	GPS,
	CAMERA
};

class SensorMessage
{
public:
	SensorMessage(const std::string message);
	bool parse();
	std::string getDeviceID() const;
	SensorType getSensorType() const;
	void printValues() const;
	void printMessage() const;
	void logMessage() const;

private:
	std::string message;
	std::string deviceid;
	std::string sensordata;
	size_t seqno;
	std::string timestamp; // string for now
	size_t datasize;
	enum SensorType sensortype;
};

#endif