# Where to put the binary files
OUTPUT_DIR = bin

CXX = g++
# Where to find the test cases
TEST_DIR = testing

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11 -O3

all: bufferManagerTest

clean:
	rm *.o bin/* data/*

bufferManagerTest: BufferManager.o BufferFrame.o buffertest.o
	$(CXX) $(CXXFLAGS) -lpthread $^ -o $(OUTPUT_DIR)/$@
BufferManager.o: buffer/BufferManager.cpp buffer/BufferManager.hpp buffer/BufferFrame.hpp
	$(CXX) $(CXXFLAGS) -c $<
BufferFrame.o: buffer/BufferFrame.cpp buffer/BufferFrame.hpp
	$(CXX) $(CXXFLAGS) -c $<
buffertest.o: testing/buffertest.cpp buffer/BufferManager.hpp buffer/BufferFrame.hpp
	$(CXX) $(CXXFLAGS) -c $<
