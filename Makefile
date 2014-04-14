# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = gtest-1.7.0

# Where to put the binary files
OUTPUT_DIR = bin

# Where to find the test cases
TEST_DIR = testing

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11 -O3

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = externalSort_unittest

CODE = externalSort
# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

all : $(TESTS) gen

clean :
	rm -f gtest.a gtest_main.a *.o bin/*

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) -o $@ $^

# Builds the externalSorting algorithm.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

externalSort.o : externalSorting/externalSort.cpp $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
externalSort_unittest.o : $(TEST_DIR)/externalSort_unittest.cpp $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

externalSort_unittest : externalSort.o externalSort_unittest.o gtest.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $(OUTPUT_DIR)/$@

# build the random number generator
gen: gen/gen.cpp Makefile
	g++ -g -O2 -Werror gen/gen.cpp -o $(OUTPUT_DIR)/$@
