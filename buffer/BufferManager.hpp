#ifndef H_BUFFERMANAGER
#define H_BUFFERMANAGER

#include "BufferFrame.hpp"
#include "bufferValues.hpp"
#include <cstdlib>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <list>

//Manager for all buffer frames
class BufferManager {
	private:
		std::unordered_map<uint64_t, BufferFrame> bufferFrames;
		std::unordered_map<uint64_t, int> fdForSegment;
		std::list<uint64_t> lru_list;
		const unsigned size;
		std::mutex mtx;
		uint64_t replaceCounter;
		uint64_t misses;
    public:
        BufferManager(unsigned size);
        BufferFrame& fixPage(uint64_t pageId, bool exclusive);
        void unfixPage(BufferFrame& frame, bool isDirty);
        ~BufferManager();
};
#endif

//stackoverflow double free or corruption