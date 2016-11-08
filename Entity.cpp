#include "Entity.h"

using namespace std;

Person::Person(string n, string ip, int port): name(n), IP_ADDRESS(ip), PORT_NUMBER(port), descriptor(-1) { }

Person::Person(Person& p) : name(p.name), IP_ADDRESS(p.IP_ADDRESS), PORT_NUMBER(p.PORT_NUMBER), descriptor(p.descriptor) { }

Person::~Person() { }

ostream& operator<<(ostream& os, const Person& p) {
	os << p.name << " (" << p.IP_ADDRESS << ":" << p.PORT_NUMBER << ")";
	return os;
}
