#ifndef ENTITY_H
#define ENTITY_H

#include <iostream>
#include <string>
#include <unordered_set>
#include <libical/ical.h>

using namespace std;

class Meeting;

class Person{
public:
  string  name;
  string  IP_ADDRESS;
  int     PORT_NUMBER;
  int     descriptor;

  Person(string name = "NULL", string ip = "NULL", int port = -1);
  Person(Person& p);
  ~Person();
  friend ostream& operator<<(ostream& out, const Person& obj);
};

struct Message{
  Meeting* meeting;
  unordered_set<icalperiodtype*> proposal; //preferred time intervals proposed by the host for the meeting
  unordered_set<icalperiodtype*> response; //time intervals available for an attendee
};
typedef struct Message Message;

#endif
