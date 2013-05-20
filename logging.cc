#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <vector>

#include "logging.hh"

typedef std::numeric_limits<double> dbl;

void logIncomingData(const SensorMessage& msg, const std::string prefix)
{
	std::string filename = prefix + "_incoming_" + msg.deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.sensordata << std::endl;
	fs.close();
}

void logIncomingCamData(const SensorMessage& msg, const std::string prefix)
{
	std::string filename = prefix + "_incoming_" + msg.deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.datasize << std::endl;
	fs.close();

	if (msg.sensordata == "NO_MOTION")
		return;

	std::string binfilename = prefix + "_incoming_" + msg.deviceid + ".data";
	std::ofstream binfs(binfilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	std::vector<unsigned char>::const_iterator it;
	for (it = msg.camsensordata.begin(); it != msg.camsensordata.end(); it++)
		binfs << *it;
	binfs.close();
}

void logOutgoingData(const std::string deviceid, const char* obuff, size_t datasize, double sendts)
{
	std::string filename = "server_outgoing_" + deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << std::setprecision(dbl::digits10) << sendts << "\t";
	fs.write(obuff, datasize);
	fs << std::endl;
	fs.close();
}

void logOutgoingCamData(const std::string deviceid, const char* obuff, size_t datasize, double sendts)
{
	std::string filename = "server_outgoing_" + deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << std::setprecision(dbl::digits10) << sendts << "\t" << datasize << std::endl;
	fs.close();

	std::string binfilename = "server_outgoing_" + deviceid + ".data";
	std::ofstream binfs(binfilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	binfs.write(obuff, datasize);
	binfs.close();
}
