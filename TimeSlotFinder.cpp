#include "TimeSlotFinder.h"
#include "Meeting.h"
#include "impl/icalspanlistimpl.h"
#include <unordered_set>
using namespace std;

void TimeSlotFinder::findAvailabilityForMeeting(Meeting *meeting, icalset *set) {
  if (!validateTime(*meeting->deadline)) {
    return;
  }

  // Get the current time since we only care about the user's availability from
  // now until the deadline
  const char *TZID = "/freeassociation.sourceforge.net/America/Toronto";
  icaltimezone *timezone = icaltimezone_get_builtin_timezone_from_tzid(TZID);
  icaltimetype currentTime = icaltime_current_time_with_zone(timezone);

  // Find free time blocks of time between now and the deadline
  icalspanlist *sl = icalspanlist_new(set, currentTime, *meeting->deadline);

  // Round off the start and end times of the spanlist to the nearest interval
  int interval = icaldurationtype_as_int(*meeting->duration);
  icaltimezone *utcTimezone = icaltimezone_get_utc_timezone();
  time_t sl_start = icaltime_as_timet_with_zone(sl->start, utcTimezone);
  sl_start /= interval;
  sl_start *= interval;

  time_t sl_end = icaltime_as_timet_with_zone(sl->end, utcTimezone);
  sl_end /= interval;
  sl_end *= interval;

  // Loop through the spanlist
  time_t spanduration_secs = sl_end - sl_start;
  time_t matrix_slots = spanduration_secs / interval + 1;
  for (pvl_elem itr = pvl_head(sl->spans); itr != 0; itr = pvl_next(itr)) {
    struct icaltime_span *s = (struct icaltime_span *)pvl_data(itr);

    if (s->is_busy == 0) {
      // Calculate how many units the span can be split into
      time_t offset_start = s->start / interval - sl_start / interval;
      time_t offset_end = (s->end - 1) / interval - sl_start / interval;

      // Account for integer truncation
      if (offset_end >= matrix_slots) {
        offset_end = matrix_slots - 1;
      }

      // Generate possible times
      for (time_t i = offset_start; i <= offset_end; ++i) {
        icaltimetype startTime = icaltime_from_timet_with_zone(sl_start, 0, utcTimezone);
        icaltime_adjust(&startTime, 0, 0, 0, interval * i);
        icaltimetype endTime = startTime;
        icaltime_adjust(&endTime, 7 * meeting->duration->weeks + meeting->duration->days, meeting->duration->hours, meeting->duration->minutes, meeting->duration->seconds);
        icalperiodtype *period = new icalperiodtype();
        *period = { startTime, endTime, *meeting->duration };
        meeting->possible_times.insert(period);
      }
    }
  }
}

std::unordered_set<icalperiodtype*> TimeSlotFinder::findAvailability(Meeting *meeting, icalset *set) {
  std::unordered_set<icalperiodtype*> possibleTimes;
  if (!validateTime(*meeting->deadline)) {
    exit(-1);
  }

  // Get the current time since we only care about the user's availability from
  // now until the deadline
  const char *TZID = "/freeassociation.sourceforge.net/America/Toronto";
  icaltimezone *timezone = icaltimezone_get_builtin_timezone_from_tzid(TZID);
  icaltimetype currentTime = icaltime_current_time_with_zone(timezone);

  // Find free time blocks of time between now and the deadline
  icalspanlist *sl = icalspanlist_new(set, currentTime, *meeting->deadline);

  // Round off the start and end times of the spanlist to the nearest interval
  int interval = icaldurationtype_as_int(*meeting->duration);
  icaltimezone *utcTimezone = icaltimezone_get_utc_timezone();
  time_t sl_start = icaltime_as_timet_with_zone(sl->start, utcTimezone);
  sl_start /= interval;
  sl_start *= interval;

  time_t sl_end = icaltime_as_timet_with_zone(sl->end, utcTimezone);
  sl_end /= interval;
  sl_end *= interval;

  // Loop through the spanlist
  time_t spanduration_secs = sl_end - sl_start;
  time_t matrix_slots = spanduration_secs / interval + 1;
  for (pvl_elem itr = pvl_head(sl->spans); itr != 0; itr = pvl_next(itr)) {
    struct icaltime_span *s = (struct icaltime_span *)pvl_data(itr);

    if (s->is_busy == 0) {
      // Calculate how many units the span can be split into
      time_t offset_start = s->start / interval - sl_start / interval;
      time_t offset_end = (s->end - 1) / interval - sl_start / interval;

      // Account for integer truncation
      if (offset_end >= matrix_slots) {
        offset_end = matrix_slots - 1;
      }

      // Generate possible times
      for (time_t i = offset_start; i <= offset_end; ++i) {
        icaltimetype startTime = icaltime_from_timet_with_zone(sl_start, 0, utcTimezone);
        icaltime_adjust(&startTime, 0, 0, 0, interval * i);
        icaltimetype endTime = startTime;
        icaltime_adjust(&endTime, 7 * meeting->duration->weeks + meeting->duration->days, meeting->duration->hours, meeting->duration->minutes, meeting->duration->seconds);
        icalperiodtype *period = new icalperiodtype();
        *period = { startTime, endTime, *meeting->duration };
        possibleTimes.insert(period);
      }
    }
  }
  return possibleTimes;
}


bool TimeSlotFinder::validateTime(icaltimetype t) {
  return icaltime_is_null_time(t) == false && icaltime_is_valid_time(t) == true;
}
