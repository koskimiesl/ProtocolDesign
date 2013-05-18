#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <vector>
#include "logging.hh"

typedef std::numeric_limits<double> dbl;

void logSensorData(const SensorMessage& msg)
{
	std::string filename = "server_" + msg.deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.seqno << "\t" << msg.sensordata << std::endl;
	fs.close();
}

void logCamSensorData(const SensorMessage& msg)
{
	std::string filename = "server_" + msg.deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.seqno << "\t" << msg.datasize << std::endl;
	fs.close();

	if (msg.sensordata == "NO_MOTION")
		return;

	std::string binfilename = "server_" + msg.deviceid + ".data";
	std::ofstream binfs(binfilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	std::vector<unsigned char>::const_iterator it;
	for (it = msg.camsensordata.begin(); it != msg.camsensordata.end(); it++)
		binfs << *it;
	binfs.close();
}
