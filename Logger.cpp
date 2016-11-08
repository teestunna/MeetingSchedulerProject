#include "Logger.h"
#include "Meeting.h"
#include "Entity.h"

#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
using namespace std;

mutex logMutex;

Logger::Logger(string f) : filename(f) {
  start = chrono::high_resolution_clock::now();
  lastTime = start;
}

void Logger::log(Meeting *meeting, Person *person, MessageType type, unordered_set<icalperiodtype *> *freeTimes)
{
  string msg;
  switch (type) {
    case FOUND_TIME_SLOTS:
      msg = foundTimeSlotsMessage(meeting);
      break;
    case SEND_INVITATION:
      msg = sendInvitationMessage(meeting, person);
      break;
    case RECEIVED_INVITATION:
      msg = receivedInvitationMessage(meeting);
      break;
    case SEND_INVITATION_REPLY:
      msg = sendInvitationReplyMessage(meeting);
      break;
    case RECEIVED_INVITATION_REPLY:
      msg = receivedInvitationReplyMessage(meeting, person);
      break;
    case INTERSECTION_ALL_REPLIES:
      msg = intersectionMessage(meeting, freeTimes);
      break;
    case INTERSECTION_ALL_REPLIES_AND_HOST:
      msg = intersectionHostMessage(meeting, freeTimes);
      break;
    case SENT_AWARD:
      msg = sentAwardMessage(meeting, person);
      break;
    case RECEIVED_AWARD:
      msg = receivedAwardMessage(meeting, person);
      break;
    case MTG_CONFIRMED:
      msg = meetingConfirmedMessage(meeting, person);
      break;
    case MTG_ABANDONED:
      msg = meetingAbandonedMessage(meeting, person);
      break;
    case RECEIVED_MTG_CONFIRMED:
      msg = receivedMeetingConfirmedMessage(meeting);
      break;
    case RECEIVED_MTG_ABANDONED:
      msg = receivedMeetingAbandonedMessage(meeting);
      break;

  }

  log(msg);
}

void Logger::log(string msg)
{
  logMutex.lock();
  chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
  stringstream ss;
  ss << chrono::duration<double, std::milli>(end-start).count() << " ms since program start." << endl;
  ss << chrono::duration<double, std::milli>(end-lastTime).count() << " ms since last event.";
  lastTime = end;
  writeToFile(ss.str());
  writeToFile(msg);
  logMutex.unlock();
}

void Logger::writeToFile(string msg)
{
  ofstream ofs;
  ofs.open(filename, ios_base::app | ios_base::out);
  ofs << msg + "\n";
  // Don't call close() explicitly; stream is closed when it goes out of scope
}

string Logger::periodtype_set_to_string(unordered_set<icalperiodtype *> set)
{
  stringstream ss;
  unordered_set<icalperiodtype *>::iterator it;
  for (it = set.begin(); it != set.end(); ++it) {
    ss << icalperiodtype_as_ical_string(**it) << endl;
  }
  return ss.str();
}

string Logger::foundTimeSlotsMessage(Meeting *meeting)
{
  stringstream ss;
  ss << "Found " << meeting->possible_times.size() << " open time slots in host's calendar for "
     << meeting->meeting_as_log_string() << endl;

  ss << "====FOUND TIME SLOTS====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "========================" << endl;
  return ss.str();
}

string Logger::sendInvitationMessage(Meeting *meeting, Person *person)
{
  stringstream ss;
  ss << "Inviting " << *person << " to " << meeting->meeting_as_log_string() << endl;

  ss << "====SUGGESTED TIMES FROM HOST====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "=================================" << endl;
  return ss.str();
}

string Logger::receivedInvitationMessage(Meeting *meeting)
{
  stringstream ss;
  ss << "Received invitation for " << meeting->meeting_as_log_string() << endl;

  ss << "====TIME SLOTS SENT BY HOST====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "===============================" << endl;
  return ss.str();
}

string Logger::sendInvitationReplyMessage(Meeting *meeting)
{
  stringstream ss;
  ss << "Replied to invitation for " << meeting->meeting_as_log_string() << endl;

  ss << "====SLOTS WHERE INVITEE WAS AVAILABLE====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "=========================================" << endl;
  return ss.str();
}

string Logger::receivedInvitationReplyMessage(Meeting *meeting, Person *person)
{
  stringstream ss;
  ss << "Received invitation reply from " << *person << " for " << meeting->meeting_as_log_string() << endl;

  ss << "====SLOTS WHERE INVITEE WAS AVAILABLE====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "=========================================" << endl;
  return ss.str();
}

string Logger::intersectionMessage(Meeting *meeting, unordered_set<icalperiodtype *>* freeTimes)
{
  stringstream ss;
  ss << "Found intersection of all replies for " << meeting->meeting_as_log_string() << endl;
  ss << "====INTERSECTION OF ALL REPLIES====" << endl;
  ss << periodtype_set_to_string(*freeTimes);
  ss << "===================================" << endl;
  return ss.str();
}

string Logger::intersectionHostMessage(Meeting *meeting, unordered_set<icalperiodtype *>* freeTimes)
{
  stringstream ss;
  ss << "Found intersection of host calendar and all replies for " << meeting->meeting_as_log_string() << endl;
  ss << "====INTERSECTION OF ALL REPLIES WITH HOST====" << endl;
  ss << periodtype_set_to_string(*freeTimes);
  ss << "===================================" << endl;
  return ss.str();
}

string Logger::sentAwardMessage(Meeting *meeting, Person *person)
{
  stringstream ss;
  string outcome = meeting->option == Meeting::AWARD ? "acceptance" : "rejection";
  ss << "Sent " << outcome << " proposal to " << *person << " for " << meeting->meeting_as_log_string() << endl;
  if (meeting->option == Meeting::AWARD) {
    ss << "====PROPOSED MEETING TIME====" << endl;
    ss << periodtype_set_to_string(meeting->possible_times);
    ss << "===================================" << endl;
  }
  return ss.str();
}

string Logger::receivedAwardMessage(Meeting *meeting, Person *person)
{
  stringstream ss;
  string outcome = meeting->option == Meeting::AWARD ? "acceptance" : "rejection";
  ss << "Received " << outcome << " proposal for " << meeting->meeting_as_log_string() << endl;
  if (meeting->option == Meeting::AWARD) {
    ss << "====PROPOSED MEETING TIME====" << endl;
    ss << periodtype_set_to_string(meeting->possible_times);
    ss << "===================================" << endl;
  }
  return ss.str();
}

string Logger::meetingConfirmedMessage(Meeting *meeting, Person *person)
{
  stringstream ss;
  ss << "Sent Meeting CONFIRMED message to " << *person << " for " << meeting->meeting_as_log_string() << endl;
  ss << "====SCHEDULED FOR ====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "===================================" << endl;
  return ss.str();
}

string Logger::meetingAbandonedMessage(Meeting *meeting, Person *person)
{
  stringstream ss;
  ss << "Sent Meeting ABANDONED message to " << *person << " for " << meeting->meeting_as_log_string() << endl;
  ss << "====NO LONGER SCHEDULED FOR ====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "===================================" << endl;
  return ss.str();
}

string Logger::receivedMeetingAbandonedMessage(Meeting *meeting)
{
  stringstream ss;
  ss << "Received Meeting ABANDONED message for " << meeting->meeting_as_log_string() << endl;
  ss << "====NO LONGER SCHEDULED FOR ====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "===================================" << endl;
  return ss.str();
}

string Logger::receivedMeetingConfirmedMessage(Meeting *meeting)
{
  stringstream ss;
  ss << "Received Meeting CONFIRMED message for " << meeting->meeting_as_log_string() << endl;
  ss << "====SCHEDULED FOR ====" << endl;
  ss << periodtype_set_to_string(meeting->possible_times);
  ss << "===================================" << endl;
  return ss.str();
}
