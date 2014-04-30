#include "BufferFrame.hpp"

#include <string>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#define PAGESIZE 4096

using namespace std;
BufferFrame::BufferFrame(uint64_t id, int fd)
    :
    pageId(id),
    state(State::clean),
    data(NULL),
    fd(fd),
    in_lru_list(false),
    readerCount(0)
{
    pthread_rwlock_init(&latch, nullptr);
    pageOffset = pageId & 0xffff; // 16 bit in a file are for the page offset
};
void* BufferFrame::getData() {
    //cout << "g " << reinterpret_cast<unsigned*>(data)[0] << " ";
    return data;
};
void BufferFrame::lock(bool exclusive) {

    if(data == NULL) {
        pthread_rwlock_wrlock(&latch);
        if(data == NULL) {
            //we are the first lock in here, nobody loaded data yet
            data = malloc(PAGESIZE);
            if(data == NULL) {
                cerr << "error allocating memory" << endl;
            } else {
                if(pread(fd, data, PAGESIZE, pageOffset * PAGESIZE) == -1) {
                    cerr << "error occured while reading from page " << pageId << endl;
                }
            }
        }
        unlock();
    }
    //get the normal lock
    if(exclusive) {
        pthread_rwlock_wrlock(&latch);
        //cout << "frame exlusivley locked" << endl;
    } else {
        pthread_rwlock_rdlock(&latch);
    }
};
void BufferFrame::unlock() {
    pthread_rwlock_unlock(&latch);
};
void BufferFrame::markAsDirty() {
    state = State::dirty;
};
void BufferFrame::increaseReaderCount() { //not thread safe, must be called from within a mutex
    readerCount++;
};
void BufferFrame::decreaseReaderCount() { //not thread safe, must be called from within a mutex
    readerCount--;
};
unsigned BufferFrame::getReaderCount() {
    return readerCount;
}
BufferFrame::~BufferFrame() {
    //cout << "replacing frame for page " << pageId << endl;
    if(state == State::dirty) {
        //cout << "writing back " << reinterpret_cast<unsigned*>(data)[0] << endl;
        posix_fallocate(fd, pageOffset * PAGESIZE, PAGESIZE);
        pwrite(fd, data, PAGESIZE, pageOffset * PAGESIZE);
    }
    free(data);
    pthread_rwlock_destroy(&latch);
}