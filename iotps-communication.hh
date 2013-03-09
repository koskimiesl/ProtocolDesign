/* IoT-PS Communication Protocol */

#ifndef PD_IOTPS_COMMUNICATION_HH
#define PD_IOTPS_COMMUNICATION_HH

#include <string>
#include <vector>

class CommMessage
{
  public:
	CommMessage(std::string message) : message(message) {} // incomplete
	bool Parse();
	bool SanityCheck();
	std::string createListReply();
	std::string createSubscribeReply();
	std::string createUnsubscribeReply();
	std::string createUpdatesMessage();

  private:
	std::string message;
	std::string command;
	std::string protocol;
	int deviceCount;
	std::vector<std::string> deviceIDs;
	std::string clientID;
	std::string serverID;
	size_t payloadSize; // in bytes
};

#endif
