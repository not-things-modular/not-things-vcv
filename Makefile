# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -I./include -I./dep/include
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)

# Add json-schema-validator to the build
SOURCES += $(wildcard dep/src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk



### Dependencies ###
dep:
	$(MAKE) -C dep
cleandep:
	echo $(MAKE) -C dep clean
	$(MAKE) -C dep clean

### Test dependencies ###
dep_test:
	$(MAKE) -C dep-test
dep_test_clean:
	$(MAKE) -C dep-test clean



### Unit tests (googletest) ###
BUILD_DIR=build
GTEST_TARGET = $(BUILD_DIR)/gtest_runner
GTEST_DIR ?= ./dep-test/googletest-release-1.12.1
GTEST_SRCS = $(GTEST_DIR)/googletest/src/gtest-all.cc $(GTEST_DIR)/googletest/src/gtest_main.cc $(GTEST_DIR)/googlemock/src/gmock-all.cc
GTEST_OBJS = $(BUILD_DIR)/googletest/gtest-all.o $(BUILD_DIR)/googletest/gtest_main.o $(BUILD_DIR)/googletest/gmock-all.o

TEST_DIR = test/googletest
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp) $(wildcard $(TEST_DIR)/**/*.cpp) $(wildcard $(TEST_DIR)/**/**/*.cpp)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/test/%.o, $(TEST_SRCS))

$(BUILD_DIR)/googletest/gtest%.o: $(GTEST_DIR)/googletest/src/gtest%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/googletest/include -I$(GTEST_DIR)/googletest -c $< -o $@

$(BUILD_DIR)/googletest/gmock%.o: $(GTEST_DIR)/googlemock/src/gmock%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/googlemock/include -I$(GTEST_DIR)/googlemock -I$(GTEST_DIR)/googletest/include -c $< -o $@

$(BUILD_DIR)/test/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/googlemock/include -I$(GTEST_DIR)/googlemock -I$(GTEST_DIR)/googletest/include -c $< -o $@

# If the Rack buildscript added the "-municode" flag to the CXXFLAGS (e.g. on Windows), remove that for the tests, since it will trigger a UI build instead of a commandline build.
GTEST_CXXFLAGS := $(filter-out -municode, $(CXXFLAGS))

$(GTEST_TARGET): $(TEST_OBJS) $(OBJECTS) $(GTEST_OBJS)
	$(CXX) $(GTEST_CXXFLAGS) $^ -o $(GTEST_TARGET) -pthread -L../.. -lRack -static-libgcc

test: all $(GTEST_TARGET)
# Remove any possible remaining coverage files in case the previous run was a test-coverage run (otherwise, the metrics might accumulate over runs)
	lcov --directory build --zerocounters
# Include RACK_DIR on the path to allow the Rack library to be included in the runtime environment.
	PATH=$$PATH:$(RACK_DIR) $(GTEST_TARGET)


### Code coverage ###
GCOVFLAGS = -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer -fno-elide-constructors -fno-default-inline

ifdef ARCH_WIN
	OLD_SHELL := $(SHELL)
	SHELL := /bin/bash
	LCOV_PWD := $(shell echo `pwd -W`/ | tr / \)
	SHELL = $(OLD_SHELL)
else
	LCOV_PWD := $(shell pwd)/
endif

test-coverage: CXXFLAGS := $(filter-out -O3, $(CXXFLAGS)) $(GCOVFLAGS) -lgcov
test-coverage: LDFLAGS := $(filter-out -O3, $(LDFLAGS))
test-coverage: GTEST_CXXFLAGS += $(GCOVFLAGS) -lgcov
test-coverage: LDFLAGS += $(GCOVFLAGS) -lgcov
test-coverage: test
	lcov --capture -d build/src -o build/coverage.info --include '$(LCOV_PWD)*'
	genhtml build/coverage.info -o build/coverage_report
