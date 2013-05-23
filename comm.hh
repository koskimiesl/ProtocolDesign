/* IoT-PS Communication Protocol */

#ifndef COMM_HH
#define COMM_HH

#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>

class CommMessage
{
public:
	CommMessage();
	CommMessage(const std::string message);
	bool parse();
	bool sanityCheck();
	void print();
	// Get
	std::string getCommand();
	std::string getVersion();
	std::string getClientID();
	std::string getServerID();
	std::string getTimeStamp();
	size_t getCount();
	size_t getSize();
	std::vector<std::string> getDeviceIDs();
	// Requests
	std::string createListRequest();
	std::string createSubsRequest();
	std::string createUnsubsRequest();
	// Good Replies
	std::string createListReply();
	std::string createSubscribeReply();
	std::string createUnsubscribeReply();
	std::string createUpdatesMessage();
	// Bad Replies 
	std::string createErrorReply();
	std::string createInvalidReply();
	// Update members
	void updateMessage(std::string msg);
	void updateVersion(std::string ver);
	void updateClientID(std::string cID);
	void updateServerID(std::string sID);
	void updateCount(size_t count);
	void updateSize(size_t size);
	void updateDeviceIDs(std::vector<std::string> dIdDs);
  
private:
	std::string message;
	std::string command;
	std::string version;
	std::string clientID;
	std::string serverID;
	std::string timestamp;	
	size_t count;
	std::vector<std::string> deviceIDs;
	size_t size; // in bytes
};

#endif
