// declare struct that contains state information about a connection
#ifndef STATE_HH
#define STATE_HH
#include<sys/socket.h>
#include<sys/types.h>
#include<cstring>
#include<queue>
#include<string>

enum Status{
	RC,
	CT,
	RE,
};

class State{
public:
	// data
	struct sockaddr addr;
	socklen_t len;
	unsigned short seq;
	unsigned short ack;
	enum Status status;
	std::queue<std::string> incoming;
	std::queue<std::string> outgoing;
	// functions
	bool isEqual(struct sockaddr *a);
	State();	
};

#endif
