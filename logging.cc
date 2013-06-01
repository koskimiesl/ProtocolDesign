#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

#include "logging.hh"

void logServerIncoming(const std::string dirname, const SensorMessage& msg, bool binarydata)
{
	std::string logfn = dirname + "/incoming_" + msg.deviceid + ".log";
	std::ofstream fs(logfn.c_str(), std::ios::out | std::ios::app);
	if (!binarydata)
	{
		if (msg.sensordata == "NO_MOTION")
			fs << "recv_ts: " << std::setprecision(2) << std::fixed << msg.receivets << "\t\tdatasize: " << msg.datasize << std::endl;
		else
			fs << "recv_ts: " << std::setprecision(2) << std::fixed << msg.receivets << "\t\tdata: " << msg.sensordata << std::endl;
		fs.close();
	}
	else
	{
		fs << "recv_ts: " << std::setprecision(2) << std::fixed << msg.receivets << "\t\tdatasize: " << msg.datasize << std::endl;
		fs.close();

		std::string datafn = dirname + "/incoming_" + msg.deviceid + ".data";
		std::ofstream binfs(datafn.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		std::vector<unsigned char>::const_iterator it;
		for (it = msg.camsensordata.begin(); it != msg.camsensordata.end(); it++)
			binfs << *it;
		binfs.close();
	}
}

void logServerOutgoing(const std::string dirname, const std::string clientid, const std::string deviceid, const char* obuff, size_t datasize, double sendts, bool binarydata, size_t seqno)
{
	std::string logfn = dirname + "/outgoing_client_" + clientid + "_" + deviceid + ".log";
	std::ofstream fs(logfn.c_str(), std::ios::out | std::ios::app);
	if (!binarydata)
	{
		fs << "send_ts: " << std::setprecision(2) << std::fixed << sendts << "\t\tseq_no: " << seqno << "\t\tdata: ";
		fs.write(obuff, datasize);
		fs << std::endl;
		fs.close();
	}
	else
	{
		fs << "send_ts: " << std::setprecision(2) << std::fixed << sendts << "\t\tseq_no: " << seqno << "\t\tdatasize: " << datasize << std::endl;
		fs.close();

		std::string datafn = dirname + "/outgoing_client_" + clientid + "_" + deviceid + ".data";
		std::ofstream binfs(datafn.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		binfs.write(obuff, datasize);
		binfs.close();
	}
}

void logClientIncoming(const std::string dirname, const std::string deviceid, const char* buff, size_t datasize, const std::string sensorts, double recvts, bool binarydata, size_t seqno)
{
	double sts;
	if ((sts = strtod(sensorts.c_str(), NULL)) == 0.0)
	{
		std::cerr << "Failed to convert string to double" << std::endl;
		return;
	}
	std::string logfn = dirname + "/incoming_" + deviceid + ".log";
	std::ofstream fs(logfn.c_str(), std::ios::out | std::ios::app);
	if (!binarydata)
	{
		fs << "recv_ts: " << std::setprecision(2) << std::fixed << recvts << "\tsensor_ts: " << sts << "\tdelay: " << recvts - sts << " s.\t\tseq_number: " << seqno << "\t\tdata: ";
		fs.write(buff, datasize);
		fs << std::endl;
		fs.close();
	}
	else
	{
		fs << "recv_ts: " << std::setprecision(2) << std::fixed << recvts << "\tsensor_ts: " << sts << "\tdelay: " << recvts - sts << " s.\t\tseq_number: " << seqno << "\t\tdatasize: " << datasize << std::endl;
		fs.close();

		std::string datafn = dirname + "/incoming_" + deviceid + ".data";
		std::ofstream binfs(datafn.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		binfs.write(buff, datasize);
		binfs.close();
	}
}
