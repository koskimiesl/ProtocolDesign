#include <fstream>
#include "logging.hh"

void logSensorData(const SensorMessage msg)
{
	std::string filename = "server_" + msg.deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << msg.timestamp << "\t" << msg.seqno << "\t" << msg.sensordata << std::endl;
	fs.close();
}

void logCamSensorData(const SensorMessage msg)
{
	std::string filename = "server_" + msg.deviceid + ".log";
	std::ofstream fs(filename.c_str(), std::ios::out | std::ios::app);
	fs << msg.timestamp << "\t" << msg.seqno << "\t" << msg.datasize << std::endl;
	fs.close();

	if (msg.sensordata == "NO_MOTION")
		return;

	std::string binfilename = "server_" + msg.deviceid + ".data";
	std::ofstream binfs(binfilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	binfs.write((char*)msg.camsensordata, msg.datasize);
	binfs.close();
}
