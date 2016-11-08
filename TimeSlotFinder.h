#ifndef TIME_SLOT_FINDER_H
#define TIME_SLOT_FINDER_H

#include <libical/ical.h>
#include <libical/icalss.h>
#include <libical/icalspanlist_cxx.h>
#include "Entity.h"

class Meeting;

class TimeSlotFinder {
  public:
    void findAvailabilityForMeeting(Meeting *meeting, icalset *set);
    std::unordered_set<icalperiodtype*> findAvailability(Meeting *meeting, icalset *set);
    Meeting * generateTimeSuggestions(icalset *set, icaltimetype start, icaltimetype end);
  private:
    bool validateTime(icaltimetype t);
};

#endif
