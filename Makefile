# Where to put the binary files
OUTPUT_DIR = bin

CXX = g++
# Where to find the test cases
TEST_DIR = testing

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11

OBJ = BufferFrame.o BufferManager.o Schema.o Parser.o SchemaSegment.o SPSegment.o Record.o 

all: bufferManagerTest schemaSegmentTest slottedTest BTreeTest OperatorTest

clean:
	rm *.o bin/* data/*

bufferManagerTest: BufferManager.o BufferFrame.o buffertest.o
	$(CXX) $(CXXFLAGS) -lpthread $^ -o $(OUTPUT_DIR)/$@
BufferManager.o: buffer/BufferManager.cpp buffer/BufferManager.hpp buffer/BufferFrame.hpp
	$(CXX) $(CXXFLAGS) -c $<
BufferFrame.o: buffer/BufferFrame.cpp buffer/BufferFrame.hpp buffer/bufferValues.hpp
	$(CXX) $(CXXFLAGS) -c $<
buffertest.o: testing/buffertest.cpp buffer/BufferManager.hpp buffer/BufferFrame.hpp
	$(CXX) $(CXXFLAGS) -c $<


Schema.o: schema/Schema.cpp schema/Schema.hpp Makefile
	$(CXX) $(CXXFLAGS) -c $<

Record.o: record/Record.cpp record/Record.hpp Makefile
	$(CXX) $(CXXFLAGS) -c $<

Parser.o: schemaParser/Parser.cpp schemaParser/Parser.hpp Makefile
	$(CXX) $(CXXFLAGS) -c $<

SchemaSegment.o: segments/SchemaSegment.cpp segments/SchemaSegment.hpp Makefile
	$(CXX) $(CXXFLAGS) -c $<

schemaSegmentTest.o: testing/schemaSegmentTest.cpp
	$(CXX) $(CXXFLAGS) -c $<

schemaSegmentTest: schemaSegmentTest.o $(OBJ)
	$(CXX) $(CXXFLAGS) -lpthread $^ -o $(OUTPUT_DIR)/$@

slottedTest.o: testing/slottedTest.cpp buffer/bufferValues.hpp
	$(CXX) $(CXXFLAGS) -c $<

SPSegment.o: segments/SPSegment.cpp segments/SPSegment.hpp buffer/bufferValues.hpp
	$(CXX) $(CXXFLAGS) -c $<
slottedTest: slottedTest.o $(OBJ)
	$(CXX) $(CXXFLAGS) -lpthread $^ -o $(OUTPUT_DIR)/$@

BTreeTest: bTree/BTree.hpp testing/BTreeTest.cpp
	$(CXX) $(CXXFLAGS) -lpthread testing/BTreeTest.cpp $(OBJ) -o $(OUTPUT_DIR)/$@

OperatorTest: testing/OperatorTest.cpp
	$(CXX) $(CXXFLAGS) -lpthread testing/OperatorTest.cpp $(OBJ) -o $(OUTPUT_DIR)/$@