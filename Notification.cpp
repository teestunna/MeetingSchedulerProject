#include "Notification.h"
#include "Meeting.h"

#include <string>
#include <sstream>
#include <unordered_set>
#include <libical/ical.h>
using namespace std;

Notification::Notification(Meeting *m, bool s) : meeting(m), scheduled(s) { }

void Notification::display() {
  #ifdef __linux__
  stringstream ss;
  unordered_set<icalperiodtype *>::iterator it = meeting->possible_times.begin();
  if (it == meeting->possible_times.end()) return;
  icalperiodtype *period = *it;

  time_t rawtime = icaltime_as_timet(period->start);
  struct tm *timeinfo = localtime(&rawtime);
  char buffer[100];
  strftime(buffer, 100, "%A, %B %e, %Y at %r", timeinfo);

  ss << "notify-send \"‘" << meeting->topic << "’ "<< string(scheduled ? "scheduled " : "abandoned ")
  << "for " << buffer << "\"";
  system(ss.str().c_str());
  #endif
}
