#include <libical/ical.h>

#include <unordered_set>
#include <string>
using namespace std;

class Person;

class Meeting {
public:
  enum MessageOptions: int {
    POSSIBLE_TIMES,
    INVITATION,
    AWARD,
    REJECT
  };
  int meetingID;
  MessageOptions option;
  string topic;
  unordered_set<Person*> attendees;
  Person* host;
  icaldurationtype* duration;
  float priority;
  unordered_set<icalperiodtype*> possible_times;
  icaltimetype* deadline;

  Meeting();
  Meeting(int, string, unordered_set<Person*>, Person*, icaldurationtype*, float, unordered_set<icalperiodtype*>, icaltimetype*);
  ~Meeting();
  string possible_times_as_string() const;
  string topic_as_string_no_asterisks(string&);
  string topic_as_string_asterisks() const;
  string meeting_as_log_string();
  icalcomponent * to_icalcomponent();
  friend ostream& operator<<(ostream& out, const Meeting& obj);
  friend istream& operator>>(istream& in, Meeting& obj);
};
