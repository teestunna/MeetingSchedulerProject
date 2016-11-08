#include "Entity.h"
#include "Meeting.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

Meeting::Meeting() : meetingID(0), topic("Untitled Event"), host(NULL), duration(NULL), deadline(NULL) { }

Meeting::~Meeting(){
 if(host)
   delete host;
 if(duration)
   delete duration;
 if(deadline)
  delete deadline;
}

Meeting::Meeting(int id,string tp,unordered_set<Person*> att, Person* h, icaldurationtype* dur, float pr,unordered_set<icalperiodtype*> pt, icaltimetype* dl): meetingID(id), topic(tp), attendees(att), host(h), duration(dur), priority(pr), possible_times(pt), deadline(dl) {}

string Meeting::possible_times_as_string() const {
  stringstream ss;
  unordered_set<icalperiodtype *>::const_iterator itr;
  for (itr = possible_times.begin(); itr != possible_times.end(); ++itr) {
    // Asterisks used to separate individual periods
    ss << icalperiodtype_as_ical_string(**itr) << "*";
  }
  string str = ss.str();
  str.pop_back(); // Delete final asterisk
  return str;
}

unordered_set<icalperiodtype *> string_to_possible_times(string str) {
  unordered_set<icalperiodtype *> possible_times;
  replace(str.begin(), str.end(), '*', ' ');
  string possible_time;
  istringstream iss(str);
  while (iss >> possible_time) {
    possible_times.insert(new icalperiodtype(icalperiodtype_from_string(possible_time.c_str())));
  }
  return possible_times;
}

string Meeting::topic_as_string_asterisks() const {
  string appendAsterisks = topic;
  replace(appendAsterisks.begin(), appendAsterisks.end(), ' ', '*');
  return appendAsterisks;
}

string Meeting::topic_as_string_no_asterisks(string& topic) {
  string removeAsterisks = topic;
  replace(removeAsterisks.begin(), removeAsterisks.end(), '*', ' ');
  return removeAsterisks;
}

ostream& operator<<(ostream& os, const Meeting& m) {
  os << m.meetingID << " ";
  os << m.topic_as_string_asterisks() << " ";
  os << m.option << " ";
  os << icaldurationtype_as_ical_string(*m.duration) << " ";
  os << m.possible_times_as_string() << " ";
  os << icaltime_as_ical_string(*m.deadline);
  return os;
}

istream& operator>>(istream &in, Meeting& obj) {
  in >> obj.meetingID;

  string meetingTopicWithoutAsterisks;
  in >> meetingTopicWithoutAsterisks;
  obj.topic = obj.topic_as_string_no_asterisks(meetingTopicWithoutAsterisks);

  int optionAsInt;
  in >> optionAsInt;
  obj.option = (Meeting::MessageOptions)optionAsInt;

  string durationStr;
  in >> durationStr;
  obj.duration = new icaldurationtype(icaldurationtype_from_string(durationStr.c_str()));

  string possibleTimesStr;
  in >> possibleTimesStr;
  obj.possible_times = string_to_possible_times(possibleTimesStr);

  string deadlineStr;
  in >> deadlineStr;
  obj.deadline = new icaltimetype(icaltime_from_string(deadlineStr.c_str()));
  return in;
}

icalcomponent * Meeting::to_icalcomponent()
{
  icalcomponent *event = icalcomponent_new_vevent();
  // Prevent setting summary to empty string since this generates an X-LIC-ERROR
  icalcomponent_set_summary(event, topic.empty() ? "Untitled Event" : topic.c_str());
  icalcomponent_set_dtstart(event, (*possible_times.begin())->start);
  icalcomponent_set_dtend(event, (*possible_times.begin())->end);
  return event;
}

string Meeting::meeting_as_log_string()
{
  stringstream ss;
  ss << "\"" << topic << "\" (duration: " << icaldurationtype_as_ical_string(*duration) << ", "
     << "deadline " << icaltime_as_ical_string(*deadline) << ")";
  return ss.str();
}
