#include <string>
#include <unordered_set>
#include <libical/ical.h>
#include <ctime>
#include <chrono>
using namespace std;

class Meeting;
class Person;

class Logger {
public:
  enum MessageType: int {
    FOUND_TIME_SLOTS,
    SEND_INVITATION,
    RECEIVED_INVITATION,
    SEND_INVITATION_REPLY,
    RECEIVED_INVITATION_REPLY,
    INTERSECTION_ALL_REPLIES,
    INTERSECTION_ALL_REPLIES_AND_HOST,
    SENT_AWARD,
    RECEIVED_AWARD,
    MTG_CONFIRMED,
    MTG_ABANDONED,
    RECEIVED_MTG_CONFIRMED,
    RECEIVED_MTG_ABANDONED
  };
  Logger(string filename);
  void log(Meeting *meeting, Person *person, MessageType type, unordered_set<icalperiodtype *>* freeTimes = 0);
  void log(string msg);

private:
  chrono::high_resolution_clock::time_point start;
  chrono::high_resolution_clock::time_point lastTime;
  string filename;
  string foundTimeSlotsMessage(Meeting *meeting);
  string sendInvitationMessage(Meeting *meeting, Person *person);
  string receivedInvitationMessage(Meeting *meeting);
  string sendInvitationReplyMessage(Meeting *meeting);
  string receivedInvitationReplyMessage(Meeting *meeting, Person *person);
  string intersectionMessage(Meeting *meeting, unordered_set<icalperiodtype *>* freeTimes);
  string intersectionHostMessage(Meeting *meeting, unordered_set<icalperiodtype *>* freeTimes);
  void writeToFile(string msg);
  string periodtype_set_to_string(unordered_set<icalperiodtype *> set);
  string sentAwardMessage(Meeting *meeting, Person *person);
  string receivedAwardMessage(Meeting *meeting, Person *person);
  string meetingConfirmedMessage(Meeting *meeting, Person *person);
  string meetingAbandonedMessage(Meeting *meeting, Person *person);
  string receivedMeetingConfirmedMessage(Meeting *meeting);
  string receivedMeetingAbandonedMessage(Meeting *meeting);
};
