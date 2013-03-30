// declare struct that contains state information about a connection
#ifndef STATE_HH
#define STATE_HH

enum Status{
	RC,
	CT,
	RE,
};

class State{
public:
	unsigned short seq;
	unsigned short ack;
	enum Status status;
	State();	
};

#endif
