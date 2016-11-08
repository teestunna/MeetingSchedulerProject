#include "CompareTimeSets.h"
#include "Meeting.h"

void CompareTimeSets::CompareSets(Meeting* meeting,  icalset *set, std::unordered_set<icalperiodtype *>* free_times){
	TimeSlotFinder finder;
	std::unordered_set<icalperiodtype*> possibleTimes = finder.findAvailability(meeting, set);
	std::unordered_set<std::string> set1, set2, set3;

	for (std::unordered_set<icalperiodtype *>::iterator it = meeting->possible_times.begin();
       it != meeting->possible_times.end();
       ++it) {
       	std::string s = icalperiodtype_as_ical_string(**it);
       	set1.insert(s);
    }

    for (std::unordered_set<icalperiodtype *>::iterator it = possibleTimes.begin();
       it != possibleTimes.end();
       ++it) {
       	std::string s = icalperiodtype_as_ical_string(**it);
       	set2.insert(s);
    }

	unordered_set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(set3, set3.begin()));
	for (std::unordered_set<std::string>::iterator it = set3.begin();
       it != set3.end();
       ++it) {
       	icalperiodtype p = icalperiodtype_from_string((*it).c_str());
       	icalperiodtype *period = new icalperiodtype(p);
       	free_times->insert(period);
    }
}


void CompareTimeSets::findIntersection(std::unordered_set<icalperiodtype *>* a, std::unordered_set<icalperiodtype *>* b, std::unordered_set<icalperiodtype *> *result){
  std::unordered_set<std::string> set1, set2, set3;

  for (std::unordered_set<icalperiodtype *>::iterator it = a->begin();
       it != a->end();
       ++it) {
        std::string s = icalperiodtype_as_ical_string(**it);
        set1.insert(s);
    }

    for (std::unordered_set<icalperiodtype *>::iterator it = b->begin();
       it != b->end();
       ++it) {
        std::string s = icalperiodtype_as_ical_string(**it);
        set2.insert(s);
    }

  result->clear();

  unordered_set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(set3, set3.begin()));
  for (std::unordered_set<std::string>::iterator it = set3.begin();
       it != set3.end();
       ++it) {
        icalperiodtype p = icalperiodtype_from_string((*it).c_str());
        icalperiodtype *period = new icalperiodtype(p);
        result->insert(period);
    }
}

