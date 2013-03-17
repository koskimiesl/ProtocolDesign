#include "comm.hh"

CommMessage::CommMessage(){
	message = "";
	command = "";
	clientID = "";
	serverID = "";
	version = "";
	count = 0;
	size = 0;
}

CommMessage::CommMessage(std::string message):message(message){
	command = "";
	clientID = "";
	serverID = "";
	version = "";
	count = 0;
	size = 0;
}

bool CommMessage::parse(){
	char tempa[30],tempb[30];
	std::stringstream ss(message);
	std::string line;
	while(std::getline(ss,line)){
		if(sscanf(line.c_str(),"%s IoTPS\\%s",tempa,tempb) == 2){
			command = tempa;
			version = tempb;		
			continue;
		}
		else if(sscanf(line.c_str(),"ClientID: %s",tempa) == 1){
			clientID = tempa;
			continue;
		}
		else if(sscanf(line.c_str(),"ServerID: %s",tempa) == 1){
			serverID = tempa;
			continue;		
		}
		else if(sscanf(line.c_str(),"DeviceID: %s",tempa) == 1){
			deviceIDs.push_back(tempa);
			continue;		
		}
		else if(sscanf(line.c_str(),"Count: %s",tempa) == 1){
			count = (size_t)strtol(tempa,NULL,10);
			continue;
		}
		else if(sscanf(line.c_str(),"Size: %s",tempa) == 1){
			size = (size_t)strtol(tempa,NULL,10);
			continue;	
		}	
	}
	return true;
}

bool CommMessage::sanityCheck(){
	// check for errors 
}

void CommMessage::print(){
	std::vector<std::string>::iterator itr;
	std::cout<<"Command: "<<command<<std::endl<<"Version: "<<version<<std::endl<<"ClientId: "<<clientID<<std::endl \
	<<"ServerID: "<<serverID<<std::endl<<"Count: "<<count<<std::endl<<"Size: "<<size<<std::endl;
	for(itr = deviceIDs.begin();itr != deviceIDs.end();itr++)
		std::cout<<*itr<<std::endl;
}

//Updates
void CommMessage::updateVersion(std::string ver){
	version = ver;
}

void CommMessage::updateClientID(std::string cID){
	clientID = cID;
}

void CommMessage::updateServerID(std::string sID){
	serverID = sID;
}

void CommMessage::updateCount(size_t c){
	count = c;
}

void CommMessage::updateSize(size_t s){
	size = s;
}

void CommMessage::updateDeviceIDs(std::vector<std::string> dIDs){
	std::vector<std::string>::iterator itr;
	for(itr = dIDs.begin();itr != dIDs.end();itr++)
		deviceIDs.push_back(*itr);
}

//Requests
std::string CommMessage::createListRequest(){
	std::string str;
	str = "LIST IoTPS\\"+version+"\r\nClientID: "+clientID+"\r\n\r\n";
	return str;
}
std::string CommMessage::createSubsRequest(){
	std::string str;
	std::vector<std::string>::iterator itr;
	std::stringstream cc;
	cc >> count;	
	str = "SUBSCRIBE IoTPS\\"+version+"\r\nCount: "+cc.str()+"\r\n";
	for(itr = deviceIDs.begin();itr != deviceIDs.end();itr++)
		str+="DeviceID: "+(*itr)+"\r\n";
	str+="ClientID: "+clientID+"\r\n\r\n";
	return str;
}
std::string CommMessage::createUnsubsRequest(){
	std::string str;
	std::vector<std::string>::iterator itr;
	std::stringstream cc;
	cc >> count;	
	str = "UNSUBSCRIBE IoTPS\\"+version+"\r\nCount: "+cc.str()+"\r\n";
	for(itr = deviceIDs.begin();itr != deviceIDs.end();itr++)
		str+="DeviceID: "+(*itr)+"\r\n";
	str+="ClientID: "+clientID+"\r\n\r\n";
	return str;
	
}

//Good Replies
std::string CommMessage::createListReply(){
	std::string str;
	std::stringstream ss;
	ss >> size;
	str = "OK IoTPS\\"+version+"\r\nSize: "+ss.str()+"\r\nServerID: "+serverID+"\r\n\r\n";
	return str;
}
std::string CommMessage::createSubscribeReply(){
	std::string str;
	std::stringstream ss,cc;
	ss >> size;
	cc >> count;
	std::vector<std::string>::iterator itr;	
	str = "OK IoTPS\\"+version+"\r\nCount: "+cc.str()+"\r\nSize: "+ss.str()+"\r\n";
	for(itr = deviceIDs.begin();itr != deviceIDs.end();itr++)
		str+="DeviceID: "+(*itr)+"\r\n";
	str+="ServerID: "+serverID+"\r\n\r\n";
	return str;
}
std::string CommMessage::createUnsubscribeReply(){
	std::string str;
	std::vector<std::string>::iterator itr;	
	std::stringstream cc;
 	cc >> count;
	str = "OK IoTPS\\"+version+"\r\nCount: "+cc.str()+"\r\n";
	for(itr = deviceIDs.begin();itr != deviceIDs.end();itr++)
		str+="DeviceID: "+(*itr)+"\r\n";
	str+="ServerID: "+serverID+"\r\n\r\n";
	return str;
}
std::string CommMessage::createUpdatesMessage(){
	std::string str;
	std::vector<std::string>::iterator itr;	
	std::stringstream ss,cc;
	ss >> size;
	cc >> count;
	str = "UPDATES IoTPS\\"+version+"\r\nCount: "+cc.str()+"\r\nSize: "+ss.str()+"\r\n";
	for(itr = deviceIDs.begin();itr != deviceIDs.end();itr++)
		str+="DeviceID: "+(*itr)+"\r\n";
	str+="ServerID: "+serverID+"\r\n\r\n";
	return str;
}


//Bad Replies 
std::string CommMessage::createErrorReply(){
	std::string str;
	str = "ERROR IoTPS\\"+version+"\r\nServerID: "+serverID+"\r\n\r\n";
	return str;
}
std::string CommMessage::createInvalidReply(){
	std::string str;
	str = "INVALID IoTPS\\"+version+"\r\nServerID: "+serverID+"\r\n\r\n";
	return str;
}

