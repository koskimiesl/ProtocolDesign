#ifndef SENSORMSG_HH
#define SENSORMSG_HH

#include <string>
#include <vector>

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
	SensorMessage(const std::string message, double receivets);
	bool parse();
	bool parseCamData(const std::string data, size_t nbytes);
	std::string parseValue(const std::string fieldname) const;
	unsigned int hexToUInt(const std::string hexstr) const;
	unsigned int escSeqToUInt(const std::string escseqstr) const;
	void camDataToArray(unsigned char* camdata) const;
	void printValues() const;
	void printMessage() const;

	std::string message;
	std::string deviceid;
	std::string sensordata;
	std::vector<unsigned char> camsensordata;
	size_t seqno;
	std::string sensorts; // sensor timestamp
	double receivets; // receive timestamp
	size_t datasize;
	enum SensorType sensortype;
};

#endif
