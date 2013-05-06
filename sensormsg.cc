#include "sensormsg.hh"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

SensorMessage::SensorMessage(const char* message) : message(message)
{ }

bool SensorMessage::parse()
{
	if (message == NULL)
		return false;
	std::string msg(message);
	const std::string delimiter = "'";
	size_t name_start, name_end, value_start, value_end; // positions of 4 successive delimiters
	name_start = msg.find(delimiter, 0);
	name_end = msg.find(delimiter, name_start + 1);
	value_start = msg.find(delimiter, name_end + 1);
	value_end = msg.find(delimiter, value_start + 1);
	if (name_start == std::string::npos || name_end == std::string::npos ||
		value_start == std::string::npos || value_end == std::string::npos)
	{
		std::cout << "Delimiter not found" << std::endl;
		return false;
	}
	std::string fieldname = msg.substr(name_start + 1, name_end - name_start - 1);
	std::string value = msg.substr(value_start + 1, value_end - value_start - 1);
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
		size_t continuepos = value_end + 1; // position to continue searching
		while (n < 4) // message includes 4 more fields and values
		{
			name_start = msg.find(delimiter, continuepos);
			name_end = msg.find(delimiter, name_start + 1);
			value_start = msg.find(delimiter, name_end + 1);
			value_end = msg.find(delimiter, value_start + 1);
			if (name_start == std::string::npos || name_end == std::string::npos ||
				value_start == std::string::npos || value_end == std::string::npos)
			{
				std::cout << "Delimiter not found" << std::endl;
				return false;
			}
			fieldname = msg.substr(name_start + 1, name_end - name_start - 1);
			value = msg.substr(value_start + 1, value_end - value_start - 1);
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
			continuepos = value_end + 1;
			n++;
		}
	}
	else // parse camera message (not ready)
	{
		std::cout << "Parsing camera data" << std::endl;
		deviceid = value;
		sensortype = CAMERA;
		size_t pos;

		// check data size
		if ((pos = msg.find("data_size")) == std::string::npos)
		{
			std::cout << "data_size field not found" << std::endl;
			return false;
		}
		name_end = msg.find(delimiter, pos);
		value_start = msg.find(delimiter, name_end + 1);
		value_end = msg.find(delimiter, value_start + 1);
		if (name_end == std::string::npos || value_start == std::string::npos ||
			value_end == std::string::npos)
		{
			std::cout << "Delimiter not found" << std::endl;
			return false;
		}
		value = msg.substr(value_start + 1, value_end - value_start - 1);
		std::istringstream valueiss(value);
		valueiss >> datasize;

		// check sensor data
		if ((pos = msg.find("sensor_data")) == std::string::npos)
		{
			std::cout << "sensor_data field not found" << std::endl;
			return false;
		}
		name_end = msg.find(delimiter, pos);
		value_start = msg.find(delimiter, name_end + 1);
		value_end = msg.find(delimiter, value_start + 1);
		if (name_end == std::string::npos || value_start == std::string::npos ||
			value_end == std::string::npos)
		{
			std::cout << "Delimiter not found" << std::endl;
			return false;
		}
		value = msg.substr(value_start + 1, value_end - value_start - 1);
		if (value == "NO_MOTION")
		{
			std::cout << "camera no motion" << std::endl;
			sensordata = value;
			return true;
		}

		// write camera data to a file
		const char* data_start = strstr(message, "sensor_data") + 15; // should be pointer to data start?
		size_t i;
		std::ofstream fs("camdata.data", std::ios::out | std::ios::binary | std::ios::app);
		fs.write(data_start, datasize);
		fs.flush();
		fs.close();
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
