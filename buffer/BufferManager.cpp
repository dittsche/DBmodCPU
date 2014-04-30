#include "BufferManager.hpp"

#include <memory>
#include <stdexcept> 
#include <iostream>
#include <unistd.h>
#include <stdlib.h> 
#include <fcntl.h>

using namespace std;

BufferManager::BufferManager(unsigned size)
    :
    size(size),
    replaceCounter(0),
    misses(0)
{
    bufferFrames.reserve(size);
};
BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {

    mtx.lock();
    if(bufferFrames.count(pageId) == 0) {

        if(bufferFrames.size() >= size) {
            //push one page out
            if(!lru_list.empty()) {
                uint64_t& ref = lru_list.front();
                uint64_t pageIdToReplace = ref;
                lru_list.pop_front();
                bufferFrames.erase(pageIdToReplace); // page is in lru list, so nobody has currently fixed the page
            } else {
                //no pages to push out, no space left for new pages and current page not in buffer, so throw error
                cerr << "not enough space assigned for the buffer manager, cannot fixPage. Shutting down";
                exit(1);
            }
        }

        uint64_t segmentId = pageId >> 16;
        int fd;
        if(fdForSegment.count(segmentId) == 0) {
            //segment has not been opened before
            fd = open(("data/" + to_string(segmentId)).c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
            fdForSegment[segmentId] = fd;
        } else {
            fd = fdForSegment[segmentId];
        }
        bufferFrames.emplace(piecewise_construct, forward_as_tuple(pageId), make_tuple(pageId, fd));
    }
    BufferFrame& frame = bufferFrames.at(pageId); //reference is valid even after rehash (even though we should never rehash anyway)
    if(frame.in_lru_list) {
        //remove it from the list;
        list<uint64_t>::iterator it = frame.lru_position;
        lru_list.erase(it);
        frame.in_lru_list = false;
    }
    frame.increaseReaderCount();
    mtx.unlock(); //so we can let the next thread in
    frame.lock(exclusive); //if we get blocked here, other threads can still enter fixPage()

    return frame;
};
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    mtx.lock();
    frame.decreaseReaderCount();
    if(isDirty) {
        frame.markAsDirty();
    }
    if(frame.getReaderCount() == 0) {
        //add to lru;
        //cout << "pageid " << frame.pageId << endl;
        lru_list.push_back(frame.pageId);
        frame.lru_position = --lru_list.end();
        frame.in_lru_list = true; //we cant know if iterator is valid or not
    }
    frame.unlock();
    mtx.unlock();
};

BufferManager::~BufferManager() {
    //manually delete all bufferframes, so they write back before we close the fd's
    //cout << "#misses: " << misses << endl;
    //cout << "#pages replace: " << replaceCounter << endl;
    bufferFrames.clear();
    for(auto fd : fdForSegment) {
        close(fd.second);
    }
};