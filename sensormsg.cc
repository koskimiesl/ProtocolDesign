#include "sensormsg.hh"
#include <iostream>
#include <sstream>

SensorMessage::SensorMessage(const std::string message) : message(message)
{ }

bool SensorMessage::parse()
{
	const std::string delimiter = "'";
	size_t n = 0;
	size_t first, second, third, fourth; // positions of 4 successive delimiters
	size_t continuepos = 0; // position to continue searching
	while (n < 5) // message includes 5 values
	{
		first = message.find(delimiter, continuepos);
		second = message.find(delimiter, first + 1);
		third = message.find(delimiter, second + 1);
		fourth = message.find(delimiter, third + 1);
		if (first == std::string::npos || second == std::string::npos ||
			third == std::string::npos || fourth == std::string::npos)
			return false;

		std::string fieldname = message.substr(first + 1, second - first - 1);
		std::string value = message.substr(third + 1, fourth - third - 1);
		// input string stream to help with conversions to size_t
		std::istringstream valueiss(value);

		if (fieldname == "dev_id")
		{
			deviceid = value;
			if (value.find("device") != std::string::npos)
				sensortype = DEVICE;
			else if (value.find("temp") != std::string::npos)
				sensortype = TEMPERATURE;
			else if (value.find("gps") != std::string::npos)
				sensortype = GPS;
			else if (value.find("camera") != std::string::npos)
				sensortype = CAMERA;
			else
				return false;
		}
		else if (fieldname == "sensor_data")
			sensordata = value;
		else if (fieldname == "seq_no")
			valueiss >> seqno;
		else if (fieldname == "ts")
			timestamp = value;
		else if (fieldname == "data_size")
			valueiss >> datasize;
		else
			return false;
		continuepos = fourth + 1;
		n++;
	}
	return true;
}

std::string SensorMessage::getDeviceID() const
{
	return deviceid;
}

SensorType SensorMessage::getSensorType() const
{
	return sensortype;
}

void SensorMessage::printValues() const
{
	std::cout << "Device ID: " << deviceid << std::endl;
	std::cout << "Sensor data: " << sensordata << std::endl;
	std::cout << "Seq no: " << seqno << std::endl;
	std::cout << "Timestamp: " << timestamp << std::endl;
	std::cout << "Data size: " << datasize << std::endl;
	std::cout << "Sensor type: " << sensortype << std::endl;
}

void SensorMessage::printMessage() const
{
	std::cout << message << std::endl;
}
