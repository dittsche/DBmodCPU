#ifndef BUFFERFRAME_HPP
#define BUFFERFRAME_HPP

#include <cstdint>
#include <mutex>
#include <list>
#include "pthread.h"
#include "bufferValues.hpp"

//Frame for the buffer manager
class BufferFrame{
    public:
        const uint64_t pageId;
        enum class State : unsigned {clean, dirty};
        std::list<uint64_t>::iterator lru_position;
        bool in_lru_list;
        BufferFrame(uint64_t pageId, int fd);
        ~BufferFrame();
        BufferFrame(const BufferFrame&) = delete;
        void* getData();
        void lock(bool exlusive);
        void unlock();
        void markAsDirty();
        void increaseReaderCount();
        void decreaseReaderCount();
        unsigned getReaderCount();
    private:
        pthread_rwlock_t latch;
        uint64_t lsn;
        uint64_t pageOffset;
        unsigned readerCount;
        State state;
        void* data;
        int fd;
};
#endif