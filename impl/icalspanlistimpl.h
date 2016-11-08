#ifndef ICALSPANLISTIMPL_H
#define ICALSPANLISTIMPL_H

struct icalspanlist_impl {
  pvl_list spans;
  struct icaltimetype start;
  struct icaltimetype end;
};

#endif
