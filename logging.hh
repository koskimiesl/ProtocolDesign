#ifndef LOGGING_HH
#define LOGGING_HH

#include "sensormsg.hh"

void logServerIncoming(const std::string dirname, const SensorMessage& msg, bool binarydata);

void logServerOutgoing(const std::string dirname, const std::string deviceid, const char* obuff, size_t datasize, double sendts, bool binarydata);

void logClientIncoming(const std::string dirname, const std::string deviceid, const char* buff, size_t datasize, const std::string sensorts, double recvts, bool binarydata);

#endif
