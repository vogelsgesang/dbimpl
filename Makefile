CXXFLAGS =-std=c++11 -Wall
LDFLAGS  =-Wall -Werror
LDLIBS   =-lm

.PHONY: clean all

all: bin/sort bin/generateRandomUint64File

clean:
	rm -rf build/
	rm -rf bin/

bin/sort: build/cli/sort.o
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/generateRandomUint64File: build/cli/generateRandomUint64File.o
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

#general rule in order to build the object files
build/%.o: %.cpp
	@mkdir -p $(dir $@) 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

#automatically generate Make rules for the included header files
build/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MF $@ -MM -MP -MT $@ -MT $(basename $@).o $<

#include these make rules
DEPFILES=$(patsubst %.cpp, build/%.d, $(wildcard **/*.cpp))
-include $(DEPFILES)
