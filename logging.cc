#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <vector>

#include "logging.hh"

typedef std::numeric_limits<double> dbl;

void logServerIncoming(const std::string dirname, const SensorMessage& msg, bool binarydata)
{
	std::string logfn = dirname + "/incoming_" + msg.deviceid + ".log";
	std::ofstream fs(logfn.c_str(), std::ios::out | std::ios::app);
	if (!binarydata)
	{
		if (msg.sensordata == "NO_MOTION")
			fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.datasize << std::endl;
		else
			fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.sensordata << std::endl;
		fs.close();
	}
	else
	{
		fs << std::setprecision(dbl::digits10) << msg.receivets << "\t" << msg.datasize << std::endl;
		fs.close();

		std::string datafn = dirname + "/incoming_" + msg.deviceid + ".data";
		std::ofstream binfs(datafn.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		std::vector<unsigned char>::const_iterator it;
		for (it = msg.camsensordata.begin(); it != msg.camsensordata.end(); it++)
			binfs << *it;
		binfs.close();
	}
}

void logServerOutgoing(const std::string dirname, const std::string deviceid, const char* obuff, size_t datasize, double sendts, bool binarydata)
{
	std::string logfn = dirname + "/outgoing_" + deviceid + ".log";
	std::ofstream fs(logfn.c_str(), std::ios::out | std::ios::app);
	if (!binarydata)
	{
		fs << std::setprecision(dbl::digits10) << sendts << "\t";
		fs.write(obuff, datasize);
		fs << std::endl;
		fs.close();
	}
	else
	{
		fs << std::setprecision(dbl::digits10) << sendts << "\t" << datasize << std::endl;
		fs.close();

		std::string datafn = dirname + "/outgoing_" + deviceid + ".data";
		std::ofstream binfs(datafn.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		binfs.write(obuff, datasize);
		binfs.close();
	}
}

void logClientIncoming(const std::string dirname, const std::string deviceid, const char* buff, size_t datasize, const std::string sensorts, double recvts, bool binarydata)
{
	std::string logfn = dirname + "/incoming_" + deviceid + ".log";
	std::ofstream fs(logfn.c_str(), std::ios::out | std::ios::app);
	if (!binarydata)
	{
		fs << std::setprecision(dbl::digits10) << recvts << "\t" << sensorts << "\t";
		fs.write(buff, datasize);
		fs << std::endl;
		fs.close();
	}
	else
	{
		fs << std::setprecision(dbl::digits10) << recvts << "\t" << sensorts << "\t" << datasize << std::endl;
		fs.close();

		std::string datafn = dirname + "/incoming_" + deviceid + ".data";
		std::ofstream binfs(datafn.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		binfs.write(buff, datasize);
		binfs.close();
	}
}
