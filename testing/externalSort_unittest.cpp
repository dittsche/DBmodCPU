#include "../externalSorting/externalSort.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

std::string outputfile, inputfile;
uint64_t memoryBuffer = 0;


off_t fsize(std::string &filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) == 0)
        return st.st_size;

    return -1; 
}

TEST(externalSort, testCorrectSorting) {
    int input, output, error;
    if((input = open(inputfile.c_str(), O_RDONLY)) < 0) {
        std::cerr << "cannot open input file: " << strerror(errno) << std::endl;
        return;
    }
    uint64_t input_filesize = fsize(inputfile);
    if((output = open(outputfile.c_str(), O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
        std::cerr << "cannot open output file: " << strerror(errno) << std::endl;
        return;
    } else if((error = posix_fallocate(output, 0, input_filesize))) {
	    std::cout << "error: " << strerror(error) << std::endl;
	    exit(1);
    }
    externalSort(input, input_filesize / sizeof(uint64_t) , output, memoryBuffer);
    
    
    close(input);
    close(output);
	
    std::cout << "starting to check output" << std::endl;
    if((output = open(outputfile.c_str(), O_RDONLY)) < 0) {
        std::cerr << "cannot open output file for checking: " << strerror(errno) << std::endl;
        return;
    }
    uint64_t number = 0, lastnumber = 0;

    while(read(output, &number, sizeof(uint64_t)) > 0) {
        EXPECT_GE(number, lastnumber);
        lastnumber = number;
    }
    close(output);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    if (argc < 4) {
		std::cerr << "usage: " << argv[0] << " <inputFile> <outputFile> <memoryBufferInMB>" << std::endl;
		return -1;
	}
    memoryBuffer = atoi(argv[3]); // argument gives MB
    if(memoryBuffer < 1) {
        std::cerr << "Please give a bufferSize > 0MB" << std::endl;
        exit(1);
    }
    inputfile = argv[1];
    outputfile = argv[2];

    memoryBuffer = 1024 * 1024 * memoryBuffer; //convert it to Byte
    return RUN_ALL_TESTS();
}
