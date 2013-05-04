#include"state.hh"

State::State(){
	seq = 1; //make it random
	ack = 0;
	fd = 0;
	memset(&addr,0,sizeof(struct sockaddr));
	status = RC;
}

bool State::isEqual(struct sockaddr *a){
	return (memcmp(a,&addr,sizeof(struct sockaddr)) == 0);
}

bool State::isEqual(int f){
	return (f == fd);
}

void State::acK(unsigned short a){
	
}

