#ifndef COMPARE_TIME_SETS_H
#define COMPARE_TIME_SETS_H

#include "TimeSlotFinder.h"
#include <algorithm>

class CompareTimeSets{
  public:

    /** Compares the host's possible times with the attendee's possible times and returns the set of free times **/
    void CompareSets(Meeting* meeting,  icalset *set, std::unordered_set<icalperiodtype *>* free_times);
    void findIntersection(std::unordered_set<icalperiodtype *>* a, std::unordered_set<icalperiodtype *>* b, std::unordered_set<icalperiodtype *> *result);
};

/** template for finding the intersection between two unordered sets **/
template <typename InIt1, typename InIt2, typename OutIt>
OutIt unordered_set_intersection(InIt1 b1, InIt1 e1, InIt2 b2, InIt2 e2, OutIt out) {
    while (!(b1 == e1)) {
        if (!(std::find(b2, e2, *b1) == e2)) {
            *out = *b1;
            ++out;
        }

        ++b1;
    }

    return out;
}

#endif
