CXXFLAGS := -Wall -Werror -g3 -std=c++11

GEN_SRC = generator.cc
GEN_OBJECTS = $(GEN_SRC:%.cc=%.o)
GEN = $(GEN_SRC:%.cc=%.out)

SRC_FILES := $(wildcard *.cc)
SRC_FILES := $(filter-out $(GEN_SRC), $(SRC_FILES))
SRC_OBJS = $(SRC_FILES:%.cc=%.o)

DEPENDENCIES = $(wildcard *.h)

SCHEDULER = scheduler.out

TAGS = .tags

all: $(SRC_OBJS) $(SCHEDULER)

debug: CXXFLAGS += -DDO_VALGRIND
debug: CXXFLAGS += -DDEBUG_BUILD
debug: all

%.o: %.cc $(DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

generator: $(GEN)

generator.out: $(GEN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

scheduler.out: $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TAGS): $(SRC_FILES) $(DEPENDENCIES)
	ctags -f $@ $^

run_short:
	./scheduler.out ./short_test/resources.input ./short_test/jobs.input

run_long:
	./scheduler.out ./long_test/resources.input ./long_test/jobs.input

clean:
	rm -f *~ *.out *.o .tags *.outfile *.input
