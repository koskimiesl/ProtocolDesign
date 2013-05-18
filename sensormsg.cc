#include "sensormsg.hh"
#include <fstream>
#include <iostream>
#include <sstream>

SensorMessage::SensorMessage(const std::string message) : message(message)
{ }

/* Parses sensor message and returns boolean value indicating success */
bool SensorMessage::parse()
{
	// parse device id and sensor type
	std::string devid;
	if ((devid = parseValue("dev_id")).empty())
		return false;
	if (devid.find("device") != std::string::npos)
		sensortype = DEVICE;
	else if (devid.find("temp") != std::string::npos)
		sensortype = TEMPERATURE;
	else if (devid.find("gps") != std::string::npos)
		sensortype = GPS;
	else if (devid.find("camera") != std::string::npos)
		sensortype = CAMERA;
	else
	{
		std::cerr << "Unsupported sensor" << std::endl;
		return false;
	}
	deviceid = devid;

	// parse sequence number
	std::string sn;
	if ((sn = parseValue("seq_no")).empty())
		return false;
	std::istringstream sniss(sn);
	sniss >> seqno;

	// parse sensor timestamp
	if ((sensorts = parseValue("ts")).empty())
		return false;

	// parse data size
	std::string ds;
	if ((ds = parseValue("data_size")).empty())
		return false;
	std::istringstream dsiss(ds);
	dsiss >> datasize;

	if (sensortype == CAMERA) // parse camera sensor data
	{
		size_t namestart;
		if ((namestart = message.find("'sensor_data':")) == std::string::npos)
		{
			std::cerr << "Camera sensor_data field not found" << std::endl;
			return false;
		}
		const std::string delimiter = "'";
		size_t nameend, valuestart, valueend;
		if ((nameend = message.find(delimiter, namestart + 1)) == std::string::npos)
		{
			std::cerr << "Camera sensor_data field name end not found" << std::endl;
			return false;
		}
		valuestart = message.find(delimiter, nameend + 1);
		valueend = message.find(delimiter, valuestart + 1);
		if (valuestart == std::string::npos or valueend == std::string::npos)
		{
			std::cerr << "Delimiter(s) not found" << std::endl;
			return false;
		}
		std::string value = message.substr(valuestart + 1, valueend - valuestart - 1);
		if (value == "NO_MOTION")
		{
			sensordata = value;
			return true;
		}
		value = message.substr(nameend + 4);
		if (!parseCamData(value, datasize))
		{
			std::cerr << "Failed to parse camera data" << std::endl;
			return false;
		}
	}
	else // parse non-camera sensor data
	{
		if ((sensordata = parseValue("sensor_data")).empty())
			return false;
	}
	return true;
}

/* Parses given number of bytes from given string describing binary data into vector "camsensordata" */
bool SensorMessage::parseCamData(const std::string data, size_t nbytes)
{
	std::string::const_iterator it = data.begin();
	size_t i = 0;
	while (it != data.end() and i < nbytes)
	{
		if (*it == '\\') // backslash found
		{
			if (it + 1 != data.end() and *(it + 1) == 'x') // hex prefix found
			{
				if (it + 2 != data.end() and it + 3 != data.end()) // hex number found
				{
					std::string hex(it + 2, it + 4);
					camsensordata.push_back(hexToUInt(hex));
					it = it + 4;
				}
				else
				{
					std::cerr << "Hex number not found after prefix" << std::endl;
					return false;
				}
			}
			else if (it + 1 != data.end()) // escape sequence found
			{
				std::string escseq(it, it + 2);
				camsensordata.push_back(escSeqToUInt(escseq));
				it = it + 2;
			}
			else
			{
				std::cerr << "No character found after backslash" << std::endl;
				return false;
			}
		}
		else
		{
			camsensordata.push_back(*it);
			it++;
		}
		i++;
	}
	if (i != nbytes)
	{
		std::cerr << "Wrong number of bytes read" << std::endl;
		return false;
	}
	return true;
}

/* Parses value as a string from the given field in the message */
std::string SensorMessage::parseValue(const std::string fieldname) const
{
	size_t namestart;
	if ((namestart = message.find("'" + fieldname + "':")) == std::string::npos)
	{
		std::cerr << fieldname << " field not found" << std::endl;
		return "";
	}
	const std::string delimiter = "'";
	size_t nameend = message.find(delimiter, namestart + 1);
	size_t valuestart = message.find(delimiter, nameend + 1);
	size_t valueend = message.find(delimiter, valuestart + 1);
	if (nameend == std::string::npos or valuestart == std::string::npos
		or valueend == std::string::npos)
	{
		std::cerr << "Delimiter(s) not found" << std::endl;
		return "";
	}
	std::string value = message.substr(valuestart + 1, valueend - valuestart - 1);
	return value;
}

/* Hex number in the given string to unsigned integer value */
unsigned int SensorMessage::hexToUInt(const std::string hex) const
{
	unsigned int value;
	std::stringstream ss(hex);
	ss >> std::hex >> value;
	return value;
}

/* Escape sequence in the given string to unsigned integer value (ASCII code) */
unsigned int SensorMessage::escSeqToUInt(const std::string escseq) const
{
	if (escseq[0] != '\\' or escseq.length() != 2)
	{
		std::cerr << "Invalid escape sequence" << std::endl;
		return 0;
	}
	unsigned int value;
	switch (escseq[1])
	{
		case '\\':
		{
			value = 92;
			break;
		}
		case '\"':
		{
			value = 34;
			break;
		}
		case '\'':
		{
			value = 39;
			break;
		}
		case '0':
		{
			value = 0;
			break;
		}
		case 'a':
		{
			value = 7;
			break;
		}
		case 'b':
		{
			value = 8;
			break;
		}
		case 'f':
		{
			value = 12;
			break;
		}
		case 'n':
		{
			value = 10;
			break;
		}
		case 'r':
		{
			value = 13;
			break;
		}
		case 't':
		{
			value = 9;
			break;
		}
		case 'v':
		{
			value = 11;
			break;
		}
		default:
		{
			std::cerr << "Unidentified escape sequence" << std::endl;
			return 0;
		}
	}
	return value;
}

void SensorMessage::printValues() const
{
	std::cout << "Device ID: " << deviceid << std::endl;
	std::cout << "Sensor data: " << sensordata << std::endl;
	std::cout << "Seq no: " << seqno << std::endl;
	std::cout << "Timestamp: " << sensorts << std::endl;
	std::cout << "Data size: " << datasize << std::endl;
	std::cout << "Sensor type: " << sensortype << std::endl;
}

void SensorMessage::printMessage() const
{
	std::cout << message << std::endl;
}
