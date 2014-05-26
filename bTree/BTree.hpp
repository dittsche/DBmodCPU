#ifndef BTREE_HPP
#define BTREE_HPP

#include "../dbms/DBMS.hpp"

#include <iostream>
#include <atomic>

template<
	typename K,
	typename Comparator
>
class BTree {
private:

	struct BTreeKeyTIDPair {
		K key;
		TID tid;
	};
	struct BTreeKeyPIDPair {
		K key;
		PID pid;
	};
	struct BTreeInnerNode {
		bool isLeaf;
		uint64_t LSN;
		PID upper;
		uint64_t count;
		BTreeKeyPIDPair entry[0];
		BTreeInnerNode()
		:
			isLeaf(false),
			count(0)
		{};

	};
	struct BTreeLeafNode {
		bool isLeaf;
		uint64_t LSN;
		PID next;
		uint64_t count; 
		BTreeKeyTIDPair entry[0];
		BTreeLeafNode()
		:
			isLeaf(true),
			count(0)
		{};
	};

	BufferManager& bufferManager;
	uint64_t segmentId;
	std::atomic<uint64_t> mSize;
	uint64_t maxPage;
	PID rootNodePageId;
	Comparator smaller;
	inline PID pageIdForPage(uint64_t page) { return (segmentId << bitsForSegment) + page;};
	void insertKeyInInnerNode(K key, PID leftPid, PID rightPid, BTreeInnerNode* node );
	std::pair<BufferFrame*,BufferFrame*> split(BufferFrame* bf_node, BTreeInnerNode* parent);
	inline bool isLeaf(const BTreeInnerNode* node);
	inline bool isFull(const BTreeInnerNode* node);
	inline bool isFull(const BTreeLeafNode* node);
	inline K biggestKey(const BTreeInnerNode* node);
	inline K biggestKey(const BTreeLeafNode* node);
	inline PID getChildNodePIDForKey(const K key, const BTreeInnerNode* node);
	inline void insertKeyTidInLeaf(K key, TID tid, BTreeLeafNode* node);
	inline uint64_t findKeyInNode(const K key, const BTreeInnerNode* node) {
		uint64_t left = 0;
		uint64_t right = node->count; //first invalid slot, could be returned
		uint64_t probe = (right+left)/2;
		while(right != left) {
			probe = (right+left)/2;
			if(smaller(node->entry[probe].key, key)) {
				left = probe+1;
			} else {
				right = probe;
			}
		}
		return left;
	};
	inline uint64_t findKeyInNode(const K key, const BTreeLeafNode* node) {
		uint64_t left = 0;
		uint64_t right = node->count;
		uint64_t probe = (right+left)/2;
		while(right != left) {
			probe = (right+left)/2;
			if(smaller(node->entry[probe].key, key)) {
				left = probe+1;
			} else {
				right = probe;
			}
		}
		return left;
	};
public:
	BTree(BufferManager& bm, uint64_t segmentId);
	void initialize();
	bool insert(K key, TID tid);
	bool erase (K key);
	bool lookup (K key, TID& tid);
	size_t size() {
		return mSize;
	};
};


#include "BTree_impl.hpp"

#endif