# possible values: "build", "release"
BUILD_TYPE=release
ifneq ($(BUILD_TYPE), debug)
  ifneq ($(BUILD_TYPE), release)
    $(error Invalid build type: "$(BUILD_TYPE)")
  endif
endif

BIN_POSTFIX=
ifeq ($(BUILD_TYPE), debug)
  BIN_POSTFIX=_debug
endif

CXXFLAGS =-std=c++11 -Wall -Werror -I.
DEBUG_CXXFLAGS=-g3 -DDEBUG -O0
RELEASE_CXXFLAGS=-O3
LDFLAGS  =-Wall -Werror
LDLIBS   =-lm

.PHONY: all
all: bin/sort$(BIN_POSTFIX) bin/generateRandomUint64File$(BIN_POSTFIX)

.PHONY: test
test:
	@echo "Executing test scripts"
	@for SCRIPT in `find tests/ -name *Test.sh` ; do \
			echo -n "executing '$$SCRIPT' ... "; \
			$$SCRIPT; \
			if [ $$? -ne 0 ]; then \
				echo "failed" ; \
				exit 1; \
			else \
				echo "passed"; \
			fi \
		done;

.PHONY: clean
clean:
	rm -rf build/
	rm -rf bin/

#####################
# the actual binaries
#####################

SORT_OBJS=cli/sort.o sorting/externalSort.o utils/checkedIO.o
bin/sort$(BIN_POSTFIX): $(addprefix build/$(BUILD_TYPE)/, $(SORT_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/generateRandomUint64File$(BIN_POSTFIX): build/$(BUILD_TYPE)/cli/generateRandomUint64File.o
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

######################
# general rules
######################

#general rules in order to build the object files
build/release/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(RELEASE_CXXFLAGS) -c $< -o $@

build/debug/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_CXXFLAGS) -c $< -o $@

#automatically generate Make rules for the included header files
build/deps/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MF $@ -MM -MP -MT $@ -MT $(basename $@).o $<

#include these make rules
DEPFILES=$(patsubst %.cpp, build/deps/%.d, $(wildcard **/*.cpp))
-include $(DEPFILES)
