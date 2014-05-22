#ifndef SPSEGMENT_HPP
#define SPSEGMENT_HPP

#include "../dbms/DBMS.hpp"
#include "../record/Record.hpp"


union Slot {
	TID tid;
	struct {
		uint16_t length;
		uint16_t offset; // offset = 0 and length = 0 means no entry there
		uint16_t dummy;
		uint8_t moved;
		uint8_t direct_tuple;
	};
};
struct SP
{
	uint64_t LSN;
	unsigned slotCount;
	unsigned firstFreeSlot;
	unsigned dataStart;
	unsigned freeSpace;
	Slot slots[1];
	SP(): LSN(0),slotCount(0),firstFreeSlot(0),dataStart(PAGESIZE),freeSpace(PAGESIZE-sizeof(SP) + sizeof(Slot)) {
	};
};

class SPSegment
{
	
public:
	SPSegment(BufferManager& bm, uint64_t segmentId, uint64_t initialSize);
	TID insert(const Record& r);
	bool remove(TID tid);
	Record lookup(TID tid);
	bool update(TID tid, const Record& r);
private:
	BufferManager& bufferManager;
	uint64_t segmentId;
	uint64_t segmentSize;
	uint64_t getPageIdForTID(TID tid);
	BufferFrame& frameToInsertRecordInto(const Record& r);
	unsigned insertHelper(SP* slottedPage,const Record& r);
	void doCompaction(SP* slottedPage);
};

#endif