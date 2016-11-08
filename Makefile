CXXFLAGS = -std=c++11
EXECUTABLE = agent
OS := $(shell uname)

ifeq ($(OS),Darwin)
	INCLUDES = -I/usr/local/include
	CXX = clang++
	CXXFLAGS += -stdlib=libc++
	LDFLAGS = -Llibical/build/lib
else
	ARCH = $(shell dpkg-architecture -qDEB_BUILD_MULTIARCH)
	LDFLAGS = -Wl,-rpath /usr/local/lib/$(ARCH) -Llibical/build/lib
endif

LDLIBS = -lical -licalss -licalvcal -lical_cxx -licalss_cxx
SRCS = main.cpp Entity.cpp Agent.cpp TimeSlotFinder.cpp networking.cpp CompareTimeSets.cpp Meeting.cpp Logger.cpp Notification.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: clean debug depend extreme

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDFLAGS) $(LDLIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

clean:
	$(RM) *.o $(EXECUTABLE)

debug:	CXXFLAGS += -g
debug:	clean
debug:	all

depend: $(SRCS)
	makedepend $(INCLUDES) $^

extreme: clean
extreme: CXXFLAGS += -Wall -Wextra -Werror -pedantic-errors
extreme: all

# Aren't *you* curious? You too can have your dependencies auto-generated!
# 1. Install makedepend (`sudo apt-get install xutils-dev`)
# 2. Run `make depend` when you add new header files
# 3. Tweak the generated dependencies as necessary

# DO NOT DELETE

main.o: TimeSlotFinder.h Entity.h CompareTimeSets.h networking.h Meeting.h Logger.h Notification.h
Entity.o: Entity.h
Agent.o: Agent.h Entity.h
TimeSlotFinder.o: TimeSlotFinder.h Entity.h Meeting.h impl/icalspanlistimpl.h
networking.o: networking.h Meeting.h
CompareTimeSets.o: CompareTimeSets.h TimeSlotFinder.h Entity.h Meeting.h
Meeting.o: Entity.h Meeting.h
Logger.o: Logger.h Meeting.h Entity.h
Notification.o: Notification.h Meeting.h
