#ifndef SENSORMSG_HH
#define SENSORMSG_HH

#include <string>

class SensorMessage
{
public:
	SensorMessage(const std::string message);
	bool parse();
	void printValues() const;
	void printMessage() const;

private:
	std::string message;
	std::string deviceid;
	std::string sensordata;
	size_t seqno;
	std::string timestamp; // string for now
	size_t datasize;
};

#endif
