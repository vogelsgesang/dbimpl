# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make [all] BUILD_TYPE=debug  - makes everything; uses debug flags
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.
#   make test   - runs the test cases

CPPFLAGS = -I .
CXXFLAGS =-std=c++11 -Wall -Werror
LDFLAGS  =-Wall -Werror
LDLIBS   =-lm

# possible values: "build", "release"
BUILD_TYPE=release
ifeq ($(BUILD_TYPE), debug)
  BIN_SUFFIX=_debug
	CPP_FLAGS+= -DDEBUG
	CXX_FLAGS+= -g3 -O0
else
	ifeq ($(BUILD_TYPE), release)
		BIN_POSTFIX=
		CXXFLAGS+=-O3
	else
		$(error Invalid build type: "$(BUILD_TYPE)")
	endif
endif
OBJ_DIR=build/$(BUILD_TYPE)

.PHONY: all
all: $(addsuffix $(BIN_SUFFIX), bin/sort bin/generateRandomUint64File)

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
bin/sort$(BIN_SUFFIX): $(addprefix $(OBJ_DIR)/, $(SORT_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/generateRandomUint64File$(BIN_SUFFIX): $(OBJ_DIR)/cli/generateRandomUint64File.o
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

######################
# general rules
######################

#general rules in order to build the object files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/gtest-main.o: $(GTEST_SRCS_)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o $@

#automatically generate Make rules for the included header files
build/deps/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MF $@ -MM -MP -MT $@ -MT $(basename $@).o $<

#include these make rules
DEPFILES=$(patsubst %.cpp, build/deps/%.d, $(wildcard **/*.cpp))
-include $(DEPFILES)
