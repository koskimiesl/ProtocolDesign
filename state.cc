#include"state.hh"

State::State(){
	seq = 1; //make it random
	ack = 0;
	memset(&addr,0,sizeof(struct sockaddr));
	status = RC;
}

bool State::isEqual(struct sockaddr *a){
	if(memcmp(a,&addr,sizeof(struct sockaddr)) == 0)
		return true;
	return false;
}

