#include <iostream>
#include <list>
#include <sstream>
#include <thread>
#include <libical/ical.h>
#include <libical/icalss.h>
#include <unistd.h>
#include <mutex>
#include <sys/stat.h>
#include <signal.h>     /* signal, raise, sig_atomic_t */
#include <fcntl.h>

#include "TimeSlotFinder.h"
#include "CompareTimeSets.h"
#include "networking.h"
#include "Meeting.h"
#include "Logger.h"
#include "Notification.h"
using namespace std;

mutex invitationResponse;

const string MEETING_SCHEDULED = "MEETING_SCHEDULED";
const string MEETING_NOT_SCHEDULED = "MEETING_NOT_SCHEDULED";
const string STILL_WORKING = "STILL_WORKING";
const string FREE = "FREE";
const string NOT_FREE = "NOT_FREE";

string sendMeetingInfoToAttendeesInANiceFormat(int&, int&, int&, int&, int&, int&, unsigned int&);
list<Person *>* promptForInvitees();
Meeting * askHostForMeetingInfo();
void listen(int port, icalset* PATH);
int displayMainMenu();
void doWork(int descriptor, icalset* set);
void sendAllInvitations(list<Person *> *people, Meeting *m, icalset *set);
void invitePersonToMeeting(Person *person, Meeting *meeting, vector<Meeting *> *v);
bool findOpenTimeSlots(Meeting *m, icalset *set);
void saveMeeting(Meeting *meeting, icalset *set);
void saveMeetingForPresentation(Meeting *meeting, icalset *set);
string getTimestamp();

int listenSocket = -1;
void my_handler(int s){
           printf("Caught signal %d\n",s);
           if(listenSocket != -1){
           		close(listenSocket);
           }
           exit(0);

}

string getTimestamp();
Logger *logger;

int main(int argc, char *argv[]) {
   string filename = "log/" + getTimestamp() + ".log";
   logger = new Logger(filename);
   struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);


  if (argc < 3) {
    cout << "usage: ./agent [path-to-ics-file] [port-number]";
    return -1;
  }

  icalset *fileset = icalfileset_new_reader(argv[1]);
  if (fileset == NULL) {
    cout << "Can't create icalfileset" << endl;
    return -1;
  }

  thread t1(listen, atoi(argv[2]), fileset);
  t1.detach();

  while (displayMainMenu() != 2) {
    Meeting *meeting = askHostForMeetingInfo();
	list<Person *> *people;
    people = promptForInvitees();

    if (findOpenTimeSlots(meeting, fileset)) {
      sendAllInvitations(people, meeting, fileset);
    } else {
      continue;
    }
  }
  return 0;
}

string getTimestamp()
{
    time_t rawtime;
    time(&rawtime);
    struct tm * dt;
    char buffer [30];
    dt = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", dt);
    return std::string(buffer);
}

void sendAllInvitations(list<Person *> *people, Meeting *meeting, icalset *set) {
  vector<thread*> threads;
  vector<Meeting *> *v = new vector<Meeting *>;

  // Send out invitations on separate threads
  for (list<Person*>::iterator it = people->begin(); it != people->end(); ++it) {
    Person *person = *it;
    thread *t = new thread(invitePersonToMeeting, person, meeting, v);
    threads.push_back(t);
  }

  // Wait for replies
  vector<thread *>::iterator it;
  for (it = threads.begin(); it != threads.end(); ++it) {
    thread *t = *it;
    t->join();
  }

  // Find intersection of all replies
  CompareTimeSets c;
  vector<Meeting *>::iterator meeting_it;
  Meeting *m = v->front();
  unordered_set<icalperiodtype *> *free_times = &m->possible_times;
  for (meeting_it = v->begin(); meeting_it != v->end(); ++meeting_it) {
    c.findIntersection(free_times, &(*meeting_it)->possible_times, free_times);
  }

  logger->log(meeting, NULL, Logger::INTERSECTION_ALL_REPLIES, free_times);

  // Check host calendar to see if intersecting time slot is still available
  TimeSlotFinder finder;
  finder.findAvailabilityForMeeting(meeting, set);
  c.findIntersection(free_times, &meeting->possible_times, free_times);

  logger->log(meeting, NULL, Logger::INTERSECTION_ALL_REPLIES_AND_HOST, free_times);

  while(1) {

    bool notFree = false;

    // Send back rejections or awards
    meeting->possible_times.clear();
    if (!free_times->empty()) {
      meeting->possible_times.insert(*free_times->begin());
    }

    for (list<Person *>::iterator it = people->begin(); it != people->end(); ++it) {
      meeting->option = free_times->empty() ? Meeting::REJECT : Meeting::AWARD;
      Person *person = *it;
      logger->log(meeting, person, Logger::SENT_AWARD);
      sendMeeting(*meeting, person->descriptor);

      string *messageBackFromAttendee = new string;
      receiveMessage(*messageBackFromAttendee, person->descriptor);

      stringstream ss;
      ss << "Received a " << *messageBackFromAttendee << " reply to the proposal from " << *person << " for " << meeting->meeting_as_log_string() << endl;
      logger->log(ss.str());

      if(messageBackFromAttendee->compare(FREE) == 0) {
        // cout << endl << "We can hold a meeting" << endl << endl;
      }

      else if(messageBackFromAttendee->compare(NOT_FREE) == 0) {
        notFree = true;
      }
    }

    if (notFree) {
      icalperiodtype *temp = *free_times->begin();

      string msg = free_times->empty() ? MEETING_NOT_SCHEDULED : STILL_WORKING;
      if (msg == STILL_WORKING) {
        free_times->erase(temp);
      } else {
        Notification n(meeting, false);
        n.display();
      }

      for (list<Person *>::iterator it = people->begin(); it != people->end(); ++it) {
        Person *person = *it;
        if (msg == MEETING_NOT_SCHEDULED) {
          logger->log(meeting, person, Logger::MTG_ABANDONED);
        }
        sendMessage(msg, person->descriptor);
      }
  }
   else {
      for(list<Person *>::iterator it = people->begin(); it != people->end(); ++it) {
        Person *person = *it;
        string msg = MEETING_SCHEDULED;
        logger->log(meeting, person, Logger::MTG_CONFIRMED);
        sendMessage(msg, person->descriptor);
      }
      saveMeeting(meeting, set);
      for(list<Person *>::iterator it = people->begin(); it != people->end(); ++it) {
        Person *person = *it;
        close(person->descriptor);
      }
      Notification n(meeting, true);
      n.display();
      break;
    }
  }
}

void invitePersonToMeeting(Person *person, Meeting *meeting, vector<Meeting *> *v) {
  logger->log(meeting, person, Logger::SEND_INVITATION);

  int descriptor = -1;
  connectToServer(const_cast<char*>(person->IP_ADDRESS.c_str()), person->PORT_NUMBER, &descriptor);
  person->descriptor = descriptor;
  sendMeeting(*meeting, descriptor);

  Meeting *responseFromAttendee = new Meeting();
  receiveMeeting(*responseFromAttendee, descriptor);
  logger->log(meeting, person, Logger::RECEIVED_INVITATION_REPLY);
  invitationResponse.lock();
  v->push_back(responseFromAttendee);
  invitationResponse.unlock();
}

bool findOpenTimeSlots(Meeting *meeting, icalset *set) {
  TimeSlotFinder finder;
  finder.findAvailabilityForMeeting(meeting, set);
  if(meeting->possible_times.empty()){
    cout << "no possible times are available"<<endl<<endl;
    return false;
  }

  /* Check if the deadline for the meeting is backwards or doesn't exist */
  int deadlineCheck = icaltime_compare(*meeting->deadline, icaltime_today());

  if (meeting->deadline->month < 1 || meeting->deadline->month > 12 ||
      meeting->deadline->day < 1 || meeting->deadline->day > 31 ||
      meeting->deadline->hour < 0 || meeting->deadline->hour > 23 ||
      meeting->deadline->minute < 0 || meeting->deadline->minute > 59 ||
      meeting->deadline->second < 0 || meeting->deadline->second > 59) {
    cout << "Can't schedule meeting because an invalid date was entered." << endl;
  } else if (deadlineCheck == -1) {
    cout << "Can't schedule meeting because the deadline is in the past." << endl;
  } else if (deadlineCheck == 0) {
    cout << "Can't schedule meeting because the deadline is not far enough into the future." << endl;
  } else {
    logger->log(meeting, NULL, Logger::FOUND_TIME_SLOTS);
    return true;
  }
  return false;
}

Meeting * askHostForMeetingInfo() {
  string year, month, day, hour, minute, topic;
  int durationInMinutes;

  cout << "Please enter the latest possible date that the meeting can be held." << endl;
  cout << "  Enter desired topic for meeting: ";
  cin.get();
  getline(cin, topic);
  cout << "  Year: ";
  cin >> year;
  cout << "  Month: ";
  cin >> month;
  cout << "  Day: ";
  cin >> day;
  cout << "  Hour: ";
  cin >> hour;
  cout << "  Minute: ";
  cin >> minute;

  cout << endl << "Desired meeting length (minutes): ";
  cin >> durationInMinutes;

  stringstream ss;
  ss << year << month << day << "T" << hour << minute << "00";
  string iso_str = ss.str();

  const char* TZID = "/freeassociation.sourceforge.net/America/Toronto";
  icaltimezone *tz = icaltimezone_get_builtin_timezone_from_tzid(TZID);

  Meeting *meeting = new Meeting();
  meeting->duration = new icaldurationtype(icaldurationtype_from_int(durationInMinutes * 60));
  meeting->topic = topic;
  meeting->deadline = new icaltimetype(icaltime_from_string(iso_str.c_str()));
  meeting->option = Meeting::INVITATION;
  icaltime_set_timezone(meeting->deadline, tz);
  return meeting;
}

string sendMeetingInfoToAttendeesInANiceFormat(int& year, int& month, int& day, int& hour, int& minute, int& second, unsigned int& durationTime) {
  /* make a new instance of icaltimetype and retrieve user info */
  string intro  = "Meeting schdule information:";
  intro         += '\n';
  string sYears = "Scheduled Year: " + to_string(year);
  sYears        += '\n';
  string sMonth = "Scheduled Month: " + to_string(month);
  sMonth        += '\n';
  string sDay   = "Scheduled Day: " + to_string(day);
  sDay          += '\n';
  string sHour  = "Scheduled Hour: " + to_string(hour);
  sHour         += '\n';
  string sMin   = "Scheduled Minute: " + to_string(minute);
  sMin          += '\n';
  string sSec   = "Scheduled Seconds: " + to_string(second);
  sSec          += '\n';
  string sDur   = "Scheduled Duration Time: " + to_string(durationTime);
  sDur          += '\n';

  /* This should be the string to send to attendees over the network */
  string deployStringToAttendee = intro + sYears + sMonth + sDay + sHour + sMin + sSec + sDur;
  return deployStringToAttendee;
}

/* Prompt the user to know how many invitees to intvite to the meeting */
list<Person *> * promptForInvitees() {
  int    numberOfInvitees;
  string inviteesIPAddress;
  int portNumber;

  cout << "Enter number of invitees: ";
  cin >> numberOfInvitees;

  list<Person *> *l = new list<Person *>();

  for (int i = 0; i < numberOfInvitees; i++) {
    stringstream ss;
    ss << "Invitee " << i + 1;
    string name = ss.str();

    cout << "  Enter " << name << " IP address: ";
    cin >> inviteesIPAddress;
    cout << "  Enter " << name << " port number: ";
    cin >> portNumber;
    cout << endl;

    Person *person = new Person(name, inviteesIPAddress, portNumber);
    l->push_back(person);
  }
  return l;
}

int displayMainMenu() {
  cout << "==== Distributed Meeting Scheduler ====" << endl;
  cout << "1. Schedule a meeting" << endl;
  cout << "2. Exit" << endl;
  cout << "Enter option: ";

  int option;
  cin >> option;
  return option;
}

void listen(int port, icalset* PATH) {
	int acceptSocket = -1;
    setupListenSocket(port, &listenSocket);

    NETWORKING_LOG("Accepting connections on port " << port);

    while (1) {
      NETWORKING_LOG("Ready to accept an incoming connection!");
      acceptIncomingConnection(&listenSocket, &acceptSocket);
      NETWORKING_LOG("Connection accepted!");

      thread t1(doWork, acceptSocket, PATH);
      t1.detach();
    }
}

void doWork(int descriptor, icalset* set) {
  Meeting *meeting = new Meeting();
  receiveMeeting(*meeting, descriptor);
  logger->log(meeting, NULL, Logger::RECEIVED_INVITATION);

  if (meeting->option == Meeting::INVITATION) {
    // Find free times for invitee that are before the deadline
    CompareTimeSets handler;
    unordered_set<icalperiodtype *> free_times;
    handler.CompareSets(meeting, set, &free_times);

    // Send those times back
    meeting->option = Meeting::POSSIBLE_TIMES;
    meeting->possible_times = free_times;
    logger->log(meeting, NULL, Logger::SEND_INVITATION_REPLY);
    sendMeeting(*meeting, descriptor);
  }

  while (1) {
    // Wait for award or rejection
    Meeting *meeting2 = new Meeting();
    receiveMeeting(*meeting2, descriptor);
    string result = meeting2->option == Meeting::AWARD ? "Award" : "Rejection";
    logger->log(meeting2, NULL, Logger::RECEIVED_AWARD);

    CompareTimeSets handler;
    unordered_set<icalperiodtype *> free_times_final;
    handler.CompareSets(meeting2, set, &free_times_final);

    bool isFree = meeting2->option == Meeting::AWARD && !(free_times_final.empty());
    string msg = isFree ? FREE : NOT_FREE;

    stringstream ss;
    ss << "Sending a " + msg + " reply to the proposal for " << meeting->meeting_as_log_string() << endl;
    logger->log(ss.str());
    sendMessage(msg, descriptor);

    /* Check what we received back from the host */
    string *messageReceivedFromHost = new string;

    receiveMessage(*messageReceivedFromHost, descriptor);

    if (messageReceivedFromHost->compare(STILL_WORKING) == 0) {
      continue;
    }

    else if (messageReceivedFromHost->compare(MEETING_NOT_SCHEDULED) == 0) {
      Notification n(meeting2, false);
      n.display();
      logger->log(meeting2, NULL, Logger::RECEIVED_MTG_ABANDONED);
      break;
    }

    else if (messageReceivedFromHost->compare(MEETING_SCHEDULED) == 0) {
      Notification n(meeting2, true);
      n.display();
      logger->log(meeting2, NULL, Logger::RECEIVED_MTG_CONFIRMED);
      saveMeeting(meeting2, set);
      break;
    }
  }
  close(descriptor);
}

void saveMeeting(Meeting *meeting, icalset *set)
{
  // Open a second set because the original icalset is read-only.
  icalset *readWriteSet = icalfileset_new(icalfileset_path(set));
  if (readWriteSet == NULL) {
    cout << "saveMeeting: Failed to open icalfileset" << endl;
    return;
  }

  icalcomponent *component = meeting->to_icalcomponent();
  if (icalfileset_add_component(readWriteSet, component) == ICAL_NO_ERROR &&
      icalfileset_commit(readWriteSet) == ICAL_NO_ERROR) {
  } else {
    cout << "Failed to save meeting! " << icalerror_strerror(icalerrno) << endl;
    perror("file error");
  }

  saveMeetingForPresentation(meeting, readWriteSet);
  icalfileset_free(readWriteSet);
}

void saveMeetingForPresentation(Meeting *meeting, icalset *set)
{
  string extension = ".ics";
  string originalPath(icalfileset_path(set));
  string presentationPath = originalPath.erase(originalPath.length() - extension.length(), extension.length()) + "-presentation" + extension;

  icalfileset_options options = { O_RDWR | O_CREAT | O_TRUNC, 0644, 0, NULL };
  icalset *presentationSet = icalset_new(ICAL_FILE_SET, presentationPath.c_str(), &options);
  if (presentationSet == NULL) {
    cout << "saveMeetingForPresentation: Failed to open icalfileset" << endl;
    return;
  }

  icalcomponent *component = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);
  icalcomponent *c;
  for (c = icalset_get_first_component(set); c != 0; c = icalset_get_next_component(set)) {
    icalcomponent_add_component(component, icalcomponent_new_clone(c));
  }

  if (icalfileset_add_component(presentationSet, component) == ICAL_NO_ERROR &&
      icalfileset_commit(presentationSet) == ICAL_NO_ERROR) {
  } else {
    cout << "Failed to save meeting! " << icalerror_strerror(icalerrno) << endl;
    perror("file error presentation set");
  }
  icalfileset_free(presentationSet);
}

