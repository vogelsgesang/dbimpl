# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make [all] BUILD_TYPE=debug  - makes everything; uses debug flags
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.
#   make test   - runs the test cases

CPPFLAGS = -I . -isystem lib/gtest-1.7.0/include
CXXFLAGS =-std=c++11 -Wall -Werror -pedantic
LDFLAGS  =-Wall -Werror -pthread
LDLIBS   =-lm

# Points to the root of Google Test, relative to where this file is.
GTEST_DIR = lib/gtest-1.7.0

# possible values: "build", "release"
BUILD_TYPE=release
ifeq ($(BUILD_TYPE), debug)
  BIN_SUFFIX=_debug
  CPPFLAGS+= -DDEBUG
  CXXFLAGS+= -g3 -O0
else
  ifeq ($(BUILD_TYPE), release)
    BIN_POSTFIX=
    CXXFLAGS+=-O3
    LDFLAGS+=-O3
  else
    $(error Invalid build type: "$(BUILD_TYPE)")
  endif
endif
OBJ_DIR=build/$(BUILD_TYPE)

.PHONY: all
all: $(addsuffix $(BIN_SUFFIX), bin/sort bin/generateRandomUint64File bin/runTests bin/isSorted bin/buffertest bin/parseSchema bin/loadSchema bin/showSchema)

.PHONY: test
test: all
	@echo "=== Executing GoogleTest tests ==="
	@./bin/runTests$(BIN_SUFFIX)
	@echo "=== Executing test scripts ==="
	@for SCRIPT in `find tests/ -name *Test.sh -type f` ; do \
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

ISSORTED_OBJS=cli/isSorted.o sorting/isSorted.o utils/checkedIO.o
bin/isSorted$(BIN_SUFFIX): $(addprefix $(OBJ_DIR)/, $(ISSORTED_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/generateRandomUint64File$(BIN_SUFFIX): $(OBJ_DIR)/cli/generateRandomUint64File.o
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

BUFFER_OBJS=buffer/bufferManager.o buffer/bufferFrame.o cli/buffertest.o utils/checkedIO.o
bin/buffertest$(BIN_SUFFIX): $(addprefix $(OBJ_DIR)/, $(BUFFER_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	
PARSE_SCHEMA_OBJS=schema/RelationSchema.o schema/SchemaParser.o cli/parseSchema.o
bin/parseSchema$(BIN_SUFFIX): $(addprefix $(OBJ_DIR)/, $(PARSE_SCHEMA_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

LOAD_SCHEMA_OBJS=utils/checkedIO.o buffer/bufferFrame.o buffer/bufferManager.o schema/SchemaSegment.o \
								 schema/RelationSchema.o schema/SchemaParser.o cli/loadSchema.o
bin/loadSchema$(BIN_SUFFIX): $(addprefix $(OBJ_DIR)/, $(LOAD_SCHEMA_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

SHOW_SCHEMA_OBJS=utils/checkedIO.o buffer/bufferFrame.o buffer/bufferManager.o schema/SchemaSegment.o \
								 schema/RelationSchema.o schema/SchemaParser.o cli/showSchema.o
bin/showSchema$(BIN_SUFFIX): $(addprefix $(OBJ_DIR)/, $(SHOW_SCHEMA_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

RUNTESTS_OBJS=gtest_main.a $(patsubst %.cpp, %.o, $(shell find tests/ -iname *Test.cpp -type f)) \
              sorting/externalSort.o sorting/isSorted.o utils/checkedIO.o \
              logic/sqlBool.o buffer/bufferManager.o buffer/bufferFrame.o \
              slottedPages/spSegment.o schema/RelationSchema.o schema/SchemaParser.o \
							schema/SchemaSegment.o
bin/runTests$(BIN_SUFFIX): CPPFLAGS+= -isystem $(GTEST_DIR)/include
#the dependency on the _directory_ containing the test specifications is neccessary in
#order to handle deleted files correctly
bin/runTests$(BIN_SUFFIX): tests $(addprefix $(OBJ_DIR)/, $(RUNTESTS_OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(filter-out tests, $^) $(LDLIBS) -o $@

######################
# general rules
######################

#general rules in order to build the object files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

#######################
#rules for GTest
######################
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

$(OBJ_DIR)/gtest-all.o: $(GTEST_SRCS_)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o $@

$(OBJ_DIR)/gtest_main.o : $(GTEST_SRCS_)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o $@

$(OBJ_DIR)/gtest_main.a: $(OBJ_DIR)/gtest-all.o $(OBJ_DIR)/gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

############################
#automatically generate Make rules for the included header files
############################
build/deps/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MF $@ -MM -MP -MT $@ -MT $(basename $@).o $<

#include these make rules
DEPFILES=$(patsubst %.cpp, build/deps/%.d, $(filter-out unused/%, $(filter-out lib/%, $(wildcard **/*.cpp))))
-include $(DEPFILES)
