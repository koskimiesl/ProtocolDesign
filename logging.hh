#ifndef LOGGING_HH
#define LOGGING_HH

#include "sensormsg.hh"

void logIncomingData(const SensorMessage& msg, const std::string prefix);

void logIncomingCamData(const SensorMessage& msg, const std::string prefix);

void logIncomingData(const std::string deviceid, const char* buff, size_t datasize, std::string sensorts, double recvts);

void logIncomingCamData(const std::string deviceid, const char* buff, size_t datasize, std::string sensorts, double recvts);

void logOutgoingData(const std::string deviceid, const char* obuff, size_t datasize, double sendts);

void logOutgoingCamData(const std::string deviceid, const char* obuff, size_t datasize, double sendts);

#endif
