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
	bool parseCamData(unsigned char* camdata, const std::string data, size_t nbytes) const;
	std::string parseValue(const std::string fieldname) const;
	unsigned int hexToUInt(const std::string hexstr) const;
	unsigned int escSeqToUInt(const std::string escseqstr) const;
	void printValues() const;
	void printMessage() const;

	std::string message;
	std::string deviceid;
	std::string sensordata;
	unsigned char* camsensordata;
	size_t seqno;
	std::string timestamp; // string for now
	size_t datasize;
	enum SensorType sensortype;
};

#endif
