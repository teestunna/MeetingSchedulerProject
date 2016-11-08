#include "Agent.h"

Agent::Agent(Person* u_h) : user_host(u_h){}

Agent::~Agent(){}

void Agent::addConnectedUsers(Person* user){
	connected_users.insert(user);
}

void Agent::addConnectedUsers(std::unordered_set<Person*> users){
	for(auto it = users.cbegin(); it != users.cend(); ++it ){
		connected_users.insert(*it);
	}
}
void Agent::ScheduleMeeting(){

}

void Agent::SendMessage(__attribute__((unused)) Message* msg){

}

Message* Agent::ReceiveMessage(){
	Message* msg = NULL;
	//...
	return msg;
}
