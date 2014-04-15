#include <iostream>
#include <vector>
#include <queue>
#include <algorithm> 
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "externalSort.hpp"
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

//#define DEBUG

struct prioQueueElem {
    std::vector<uint64_t>::iterator ptr;
    uint64_t runNumber;
}; 
bool comparePrioQueueElem (prioQueueElem a, prioQueueElem b) {
    return *a.ptr > *b.ptr;
}
void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {

    uint64_t number_of_runs = (size * sizeof(uint64_t) + memSize - 1) / memSize; //get ceiling of integer division
    std::cout << "number_of_runs: " << number_of_runs <<std::endl;
    uint64_t chunkSize = (memSize / (number_of_runs + 1)) / sizeof(uint64_t);
    if(chunkSize < 2) {
        std::cerr << "Please assign a litte bit more memory, otherwise this doesnt make sense. Shutting down..." << std::endl;
        exit(1);
    }
    std::cout << "chunkSize: " << (chunkSize / (1024 * 1024)) << " MByte" << std::endl;
    uint64_t number; //buffer for one number
    uint64_t total_numbers_read = 0;
    std::vector<uint64_t> memBlock; //only use multiples of 8Byte for memory, even if we got slightly more available
    std::vector<uint64_t> elementsInRun;
    int output, error;
    std::vector<int> temp_files;
    std::string tempname = "temp";
    std::cout << "run creation phase" << std::endl;
    for(uint64_t k = 0; k < number_of_runs; k++) {
        #ifdef DEBUG
        std::cout << "sorting run #" << k << std::endl;
        #endif

        uint64_t elementsToRead = std::min(memSize/sizeof(uint64_t), (size - total_numbers_read));
        memBlock.resize(elementsToRead);
        total_numbers_read += elementsToRead;
        read(fdInput, &memBlock[0], elementsToRead * sizeof(uint64_t));

        std::sort(memBlock.begin(), memBlock.end());
        if((output = open((tempname + std::to_string(k)).c_str(),
            O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0)
        {
            std::cout << "error opening file: " << strerror(errno) << std::endl;
            exit(1);    
        } else if((error = posix_fallocate(output, 0, memBlock.size() * sizeof(uint64_t)))) {
            std::cout << "error: " << strerror(error) << std::endl;
            exit(1);
        } else {
            temp_files.push_back(output);
        }
        elementsInRun.push_back(memBlock.size());
        //write the whole run out in one operation
        write(temp_files[k], memBlock.data(), memBlock.size() * sizeof(uint64_t));
        close(temp_files[k]);
        #ifdef DEBUG
        for(auto& num: memBlock) {
            std::cout << num << std::endl;
        }
        #endif
        //clear memBlock, since we dont want to use more memory than allowed
        memBlock.clear();
    }
    //really deallocate memBlock and free the memory, see http://www.cplusplus.com/reference/vector/vector/clear/
    std::vector<uint64_t>().swap(memBlock);
    
    std::cout << "merge phase" << std::endl;
    std::vector<std::vector<uint64_t>> input_runs;
    std::vector<uint64_t> output_run;
    //open input runs and fill chunks
    for(uint64_t k = 0; k < number_of_runs; k++) {
        temp_files[k] = open((tempname + std::to_string(k)).c_str(), O_RDONLY);
        uint64_t i = 0;
        std::vector<uint64_t> run;
        uint64_t elementsToReadIntoChunk = std::min(chunkSize, elementsInRun[k]);
        run.resize(elementsToReadIntoChunk);
        elementsInRun[k] -= elementsToReadIntoChunk;
        read(temp_files[k], &run[0], elementsToReadIntoChunk * sizeof(uint64_t));
        input_runs.push_back(run);
    }
    //build prio queue over all runs
    std::priority_queue<prioQueueElem, std::vector<prioQueueElem>,decltype
        (&comparePrioQueueElem)> prioQueue(&comparePrioQueueElem);
    for(uint64_t i = 0; i < input_runs.size(); i++) {
        prioQueue.push({input_runs[i].begin(),i}); //every run contains at least one element at this point
    }
    int o = 0;
    while(!prioQueue.empty()) {
        prioQueueElem elem = prioQueue.top(); //get next elem
        prioQueue.pop(); //remove it from queue
        o++;
        #ifdef DEBUG
        std::cout << o << ":\t" << *elem.ptr << " - " << elem.runNumber << std::endl;
        #endif
        output_run.push_back(*elem.ptr); //put number in output
        if(output_run.size() == chunkSize) { //flush output buffer if full
            //std::cout << "flush output buffer" << std::endl;
            write(fdOutput, output_run.data(), output_run.size() * sizeof(uint64_t));
            output_run.clear();
        }
        elem.ptr++; //next element in chunk
        if(elem.ptr == input_runs[elem.runNumber].end()) { // read in next chunk of input
            #ifdef DEBUG
            std::cout << "reading new input chunk from run #" << elem.runNumber << std::endl;
            #endif
            uint64_t i = 0;
            input_runs[elem.runNumber].clear(); //first empty the run
            //then read up to chunkSize elements into it

            uint64_t elementsToReadIntoChunk = std::min(chunkSize, elementsInRun[elem.runNumber]);
            input_runs[elem.runNumber].resize(elementsToReadIntoChunk);
            elementsInRun[elem.runNumber] -= elementsToReadIntoChunk;
            read(temp_files[elem.runNumber], &input_runs[elem.runNumber][0], elementsToReadIntoChunk * sizeof(uint64_t));
            //then reset the iterator to the beginning
            elem.ptr = input_runs[elem.runNumber].begin(); 
        }
        //if run still contains elements, push the prioQueueElem back in
        if(input_runs[elem.runNumber].size() > 0) {
            prioQueue.push(elem);
        } else {
            #ifdef DEBUG
            std::cout << "run is empty: #" << elem.runNumber << std::endl;
            #endif
            close(temp_files[elem.runNumber]);
            remove((tempname + std::to_string(elem.runNumber)).c_str());
        }
    }
    //flush the output buffer
    write(fdOutput, output_run.data(), output_run.size() * sizeof(uint64_t));
    std::cout << "output " << o << " numbers." << std::endl;
}
