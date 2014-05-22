#include "SPSegment.hpp"
#include "../buffer/bufferValues.hpp"
#include <iostream>
#include <cassert>
using namespace std;


uint64_t getTupleOffset(TID tid) {
	return tid;
};

#define DIRECT_TUPLE_MARKER 0xff


SPSegment::SPSegment(BufferManager& bm, uint64_t segmentId, uint64_t initialSize)
:
	bufferManager(bm),
	segmentId(segmentId),
	segmentSize(initialSize)
{
};

TID SPSegment::insert(const Record& r) {
	if(r.getLen() > PAGESIZE-sizeof(SP)) {
		cerr << "tuple too big for a page. I can never insert this one" << endl;
		exit(1);
	}
	BufferFrame& bf = frameToInsertRecordInto(r);
	SP* slottedPage = (SP*)bf.getData();

	insertHelper(slottedPage, r);
	TID tid = ((bf.pageId & bitMaskForPageOffset) << bitsForSegment) + slottedPage->slotCount - 1;
	bufferManager.unfixPage(bf, true);
	
	return tid;

};
bool SPSegment::remove(TID tid) {
	BufferFrame& bf = bufferManager.fixPage(getPageIdForTID(tid), true);
	SP* slottedPage = (SP*)bf.getData();
	Slot* slot = &slottedPage->slots[tid & bitMaskForPageOffset];
	bool result;
	if(slot->direct_tuple != DIRECT_TUPLE_MARKER) {
		TID tidToDelete = slot->tid;
		bufferManager.unfixPage(bf, false);
		bool result = remove(tidToDelete);
		if(!result) {
			cout << "error, moved tuple was deleted from somewhere else" << endl;
		} else {
			BufferFrame& bf2 = bufferManager.fixPage(getPageIdForTID(tid), true);
			slottedPage = (SP*)bf2.getData();
			slot = &slottedPage->slots[tid & bitMaskForPageOffset];
			slot->direct_tuple = DIRECT_TUPLE_MARKER;
			slot->offset = 0;
			slot->length = 0;
			bufferManager.unfixPage(bf2, true);
			return result;
		}
	} else {
		if(slot->length == 0 && slot->offset == 0) {
			//tuple not there
			result = false;
			bufferManager.unfixPage(bf, false);
		} else {
			//cout << "free space before: " << slottedPage->freeSpace << endl;
			slottedPage->freeSpace += slot->length;
			//cout << "free space after: " << slottedPage->freeSpace << endl;
			slot->length = 0;
			slot->offset = 0;
			result = true;
			bufferManager.unfixPage(bf, true);
		}
	}
	
	return result;
};
Record SPSegment::lookup(TID tid) {
	//cout << "lookup TID=" << tid << " from page " << getPageIdForTID(tid) <<  endl;
	BufferFrame& bf = bufferManager.fixPage(getPageIdForTID(tid), false);
	SP* slottedPage = (SP*)bf.getData();
	Slot* slot = &slottedPage->slots[tid & bitMaskForPageOffset];
	if(slot->direct_tuple != DIRECT_TUPLE_MARKER) {
		TID lookupTid = slot->tid;
		bufferManager.unfixPage(bf, false);
		return lookup(lookupTid);
	}
	if(slot->length == 0) {
		//cout << "record does not exist <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" <<endl;
	}
	Record rec(slot->length, (char*)(slottedPage)+slot->offset);
	bufferManager.unfixPage(bf, false);
	return rec;
};
bool SPSegment::update(TID tid, const Record& r) {
	//cout << "update " << endl;
	BufferFrame& bf = bufferManager.fixPage(getPageIdForTID(tid), true);
	SP* slottedPage = (SP*)bf.getData();
	Slot* slot = &slottedPage->slots[tid & bitMaskForPageOffset];
	if(slot->direct_tuple == DIRECT_TUPLE_MARKER && r.getLen() <= slot->length) {
		//update the record in place
		memcpy((char*)slottedPage + slot->offset,r.getData(), r.getLen());
		//adjust the freeSpace and length of slot
		unsigned difference = slot->length - r.getLen();
		slottedPage->freeSpace += difference;
		slot->length = r.getLen();
		if(tid == 3014672) {
			cout << "1" << endl;
		}
		bufferManager.unfixPage(bf, true);
	} else {
		//either already redirected or redirected for the first time now
		bool deletionNecessary = false;
		TID tidToDelete;
		if(slot->direct_tuple != DIRECT_TUPLE_MARKER) {
			//cout << "tuple already redirected" << endl;
			deletionNecessary = true;
			tidToDelete = slot->tid;
		}
		bufferManager.unfixPage(bf, true);
		if(deletionNecessary) {
			remove(tidToDelete);
		}
		TID newTid = insert(r);
		BufferFrame& bf2 = bufferManager.fixPage(getPageIdForTID(tid), true);
		slottedPage = (SP*)bf2.getData();
		slot = &slottedPage->slots[tid & bitMaskForPageOffset];
		if(slot->direct_tuple == DIRECT_TUPLE_MARKER) {
			slottedPage->freeSpace += slot->length;
		}
		slot->tid = newTid;
		bufferManager.unfixPage(bf2, true);
		assert(newTid != tid);
		if(slot->direct_tuple == DIRECT_TUPLE_MARKER){
			assert(false);
		}

	}
	return true;
};
inline uint64_t SPSegment::getPageIdForTID(TID tid) {
	return (segmentId << bitsForSegment) + (tid >> bitsForSegment);
};
BufferFrame& SPSegment::frameToInsertRecordInto(const Record& r) {
	uint64_t pageId = 0;
	for(unsigned i=0;i<segmentSize;i++) {
		pageId = (segmentId << bitsForSegment) + i;
		BufferFrame& bf = bufferManager.fixPage(pageId, true);
		SP* slottedPage = (SP*)bf.getData();
		assert(slottedPage->LSN == 0);
		if(slottedPage->freeSpace >= r.getLen() + sizeof(Slot)) {
			//page has enough free space, but perhaps fragmented
			if(slottedPage->dataStart - (sizeof(SP)-sizeof(Slot)) - slottedPage->slotCount * sizeof(Slot) < r.getLen() + sizeof(Slot)) {
				doCompaction(slottedPage);
			}
			return bf;
		}
		bufferManager.unfixPage(bf, false);
	}
	//no page has enough free space, so alloc new page
	segmentSize++;
	pageId = (segmentId << bitsForSegment) + segmentSize-1;
	BufferFrame& bf = bufferManager.fixPage(pageId, true);
	SP* slottedPage = (SP*)bf.getData();
	*slottedPage = SP();
	return bf;
};

void SPSegment::doCompaction(SP* slottedPage) {
	//cout << "compaction" << endl;
	SP* newPage = (SP*)malloc(PAGESIZE);
	*newPage = SP();
	for(unsigned i=0;i<slottedPage->slotCount;i++) {
		Slot* slot = &slottedPage->slots[i];
		if(slot->direct_tuple != DIRECT_TUPLE_MARKER) {
			newPage->slots[i].tid = slot->tid;
		} else {
			newPage->dataStart -= slot->length;
			memcpy((char*)newPage + newPage->dataStart, (char*)(slottedPage)+slot->offset, slot->length);
			newPage->slots[i].length = slot->length;
			newPage->slots[i].offset = newPage->dataStart;
			newPage->slots[i].direct_tuple = DIRECT_TUPLE_MARKER;
			assert(newPage->dataStart <= PAGESIZE);
		}
		
	}
	newPage->slotCount = slottedPage->slotCount;
	newPage->freeSpace = slottedPage->freeSpace;

	memcpy(slottedPage, newPage, PAGESIZE);
	free(newPage);
};

unsigned SPSegment::insertHelper(SP* slottedPage,const Record& r) { //only call this one if you know the record fits
	slottedPage->dataStart -= r.getLen(); //set new offset for dataStart
	assert(slottedPage->dataStart <= PAGESIZE);
	assert(slottedPage->LSN == 0);
	memcpy((char*)slottedPage + slottedPage->dataStart, r.getData(), r.getLen()); //copy the record
	slottedPage->slots[slottedPage->slotCount].offset = slottedPage->dataStart;
	slottedPage->slots[slottedPage->slotCount].length = r.getLen();
	slottedPage->slots[slottedPage->slotCount].direct_tuple = DIRECT_TUPLE_MARKER;
	slottedPage->slotCount++;
	slottedPage->freeSpace = slottedPage->freeSpace - (r.getLen() + sizeof(Slot));
	return slottedPage->slotCount - 1;
}