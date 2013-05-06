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
	SensorMessage(const char* message);
	bool parse();
	void printValues() const;
	void printMessage() const;

	const char* message;
	std::string deviceid;
	std::string sensordata;
	size_t seqno;
	std::string timestamp; // string for now
	size_t datasize;
	enum SensorType sensortype;
};

#endif
