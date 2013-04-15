#include "sensormsg.hh"
#include <iostream>
#include <sstream>

SensorMessage::SensorMessage(const std::string message) : message(message)
{ }

bool SensorMessage::parse()
{
	const std::string delimiter = "'";
	size_t first, second, third, fourth; // positions of 4 successive delimiters
	first = message.find(delimiter, 0);
	second = message.find(delimiter, first + 1);
	third = message.find(delimiter, second + 1);
	fourth = message.find(delimiter, third + 1);
	if (first == std::string::npos || second == std::string::npos ||
		third == std::string::npos || fourth == std::string::npos)
	{
		std::cout << "Delimiter not found" << std::endl;
		return false;
	}
	std::string fieldname = message.substr(first + 1, second - first - 1);
	std::string value = message.substr(third + 1, fourth - third - 1);
	if (fieldname != "dev_id") // first field should be dev_id
	{
		std::cout << "Unknown message" << std::endl;
		return false;
	}
	else if (value.find("camera") == std::string::npos) // parse non-camera message
	{
		deviceid = value;
		if (value.find("device") != std::string::npos)
			sensortype = DEVICE;
		else if (value.find("temp") != std::string::npos)
			sensortype = TEMPERATURE;
		else if (value.find("gps") != std::string::npos)
			sensortype = GPS;
		else
		{
			std::cout << "Unsupported device" << std::endl;
			return false;
		}
		size_t n = 0;
		size_t continuepos = fourth + 1; // position to continue searching
		while (n < 4) // message includes 4 more fields and values
		{
			first = message.find(delimiter, continuepos);
			second = message.find(delimiter, first + 1);
			third = message.find(delimiter, second + 1);
			fourth = message.find(delimiter, third + 1);
			if (first == std::string::npos || second == std::string::npos ||
				third == std::string::npos || fourth == std::string::npos)
			{
				std::cout << "Delimiter not found" << std::endl;
				return false;
			}
			fieldname = message.substr(first + 1, second - first - 1);
			value = message.substr(third + 1, fourth - third - 1);
			// input string stream to help with conversions to size_t
			std::istringstream valueiss(value);
			if (fieldname == "sensor_data")
				sensordata = value;
			else if (fieldname == "seq_no")
				valueiss >> seqno;
			else if (fieldname == "ts")
				timestamp = value;
			else if (fieldname == "data_size")
				valueiss >> datasize;
			else
			{
				std::cout << "Unsupported field" << std::endl;
				return false;
			}
			continuepos = fourth + 1;
			n++;
		}
	}
	else // parse camera message
	{
		std::cout << "Parsing camera data" << std::endl;
		// TODO
	}
	return true;
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
