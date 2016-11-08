#ifndef AGENT_H
#define AGENT_H

#include "Entity.h"

class Agent{
public:
	Agent(Person* user_host);
	~Agent();
	void addConnectedUsers(Person* user); //add a user to the set of users connected to the host
	void addConnectedUsers(std::unordered_set<Person*> connected_users); //add a set of users to the set of users connected to the host
	void ScheduleMeeting();  //schedule a meeting
	void SendMessage(Message* msg); //send message to attendees
	Message* ReceiveMessage(); //receive message from attendee(s)
private:
	Person* user_host; //host of the agent
	std::unordered_set<Person*> connected_users; //set of users connected to the host
};
#endif
