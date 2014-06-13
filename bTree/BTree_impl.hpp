//-----------------------------------------------------------------------------
template<typename K, typename Comp>
BTree<K, Comp>::BTree(BufferManager& bm, uint64_t segmentId)
:
	bufferManager(bm),
	segmentId(segmentId),
	mSize(0),
	maxPage(0)
{
	
	
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
void BTree<K, Comp>::initialize() {
		BufferFrame& bf = bufferManager.fixPage(pageIdForPage(0), true);
		BTreeLeafNode* BTreeRootNode = (BTreeLeafNode*)bf.getData();
		rootNodePageId = pageIdForPage(0);
		*BTreeRootNode = BTreeLeafNode(); //reset root node
		bufferManager.unfixPage(bf, true);
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
bool BTree<K, Comp>::isLeaf(const BTreeInnerNode* node) {
	return node->isLeaf;
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
bool BTree<K, Comp>::isFull(const BTreeInnerNode* node) {
	uint64_t freeSpace = PAGESIZE - sizeof(BTreeInnerNode) - node->count*sizeof(BTreeKeyPIDPair);
	return freeSpace < sizeof(BTreeKeyPIDPair);
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
bool BTree<K, Comp>::isFull(const BTreeLeafNode* node) {
	uint64_t freeSpace = PAGESIZE - sizeof(BTreeLeafNode) - node->count*sizeof(BTreeKeyTIDPair);

	return freeSpace < sizeof(BTreeKeyTIDPair);
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
K BTree<K, Comp>::biggestKey(const BTreeInnerNode* node) {
	return node->entry[node->count -1].key;
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
K BTree<K, Comp>::biggestKey(const BTreeLeafNode* node) {
	return node->entry[node->count -1].key;
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
PID BTree<K, Comp>::getChildNodePIDForKey(const K key, const BTreeInnerNode* node) {
	uint64_t position = findKeyInNode(key, node);
	if(position == node->count) {
		return node->upper;
	} else {
		return node->entry[position].pid;
	}
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
void BTree<K, Comp>::insertKeyTidInLeaf(K key, TID tid, BTreeLeafNode* node) {
	uint64_t correctSlot = findKeyInNode(key, node);
	assert(node->count >= correctSlot);
	memmove(&node->entry[correctSlot+1], &node->entry[correctSlot],(node->count-correctSlot)*sizeof(BTreeKeyTIDPair));
	node->entry[correctSlot] = {key, tid};
	node->count++;
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
bool BTree<K, Comp>::insert(K key, TID tid) {
	if(lookup(key, tid)) {
		return false; //key already there
	}
	BufferFrame* bf_cur = &bufferManager.fixPage(rootNodePageId, true);
	BufferFrame* bf_par = NULL;
	BTreeInnerNode* currentNode = (BTreeInnerNode*) bf_cur->getData();
	BTreeInnerNode* parentNode = NULL;
	while(!isLeaf(currentNode)) {
		if(isFull(currentNode)) {
			//split, so we get "safe" inner pages

			std::pair<BufferFrame*,BufferFrame*> split_result = split(bf_cur, parentNode);
			BTreeInnerNode* left = (BTreeInnerNode*) split_result.first->getData();
			BTreeInnerNode* right = (BTreeInnerNode*) split_result.second->getData();

			if(smaller(key, biggestKey(left))) {	//get correct child and unlock the other one
				currentNode = left;
				bf_cur = split_result.first;
				bufferManager.unfixPage(*split_result.second, true);
			} else {
				currentNode = right;
				bf_cur = split_result.second;
				bufferManager.unfixPage(*split_result.first, true);
			}
		}

		if(parentNode != NULL) {
			bufferManager.unfixPage(*bf_par, true);
		}
		bf_par = bf_cur;
		parentNode = currentNode;
		
		PID childPID = getChildNodePIDForKey(key, currentNode);
		bf_cur = &bufferManager.fixPage(childPID, true);
		currentNode = (BTreeInnerNode*) bf_cur->getData();
	}
	BTreeLeafNode* leaf = (BTreeLeafNode*) currentNode;
	if(!isFull(leaf)) {
		insertKeyTidInLeaf(key, tid, leaf);
		bufferManager.unfixPage(*bf_cur, true);
	} else {
		std::pair<BufferFrame*,BufferFrame*> split_result = split(bf_cur, parentNode); //parent will have enough space, because we have splitted full parents along the way
		BTreeLeafNode* left = (BTreeLeafNode*) split_result.first->getData();
		BTreeLeafNode* right = (BTreeLeafNode*) split_result.second->getData();
		if(smaller(key, biggestKey(left))) {
			insertKeyTidInLeaf(key, tid, left);
		} else {
			insertKeyTidInLeaf(key, tid, right);
		}
		bufferManager.unfixPage(*split_result.first, true);
		bufferManager.unfixPage(*split_result.second, true);
	}

	if(bf_par != NULL) {
		bufferManager.unfixPage(*bf_par, true);
	}
	mSize++;
	return true;
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
std::pair<BufferFrame*, BufferFrame*> BTree<K, Comp>::split(BufferFrame* bf_node, BTreeInnerNode* parent) {
	if(parent == NULL) {
		//create new root
		BufferFrame& bf_newRoot = bufferManager.fixPage(pageIdForPage(++maxPage), true);
		parent = (BTreeInnerNode*) bf_newRoot.getData();
		rootNodePageId = bf_newRoot.pageId; //we can update the root since we have the lock to the current rootNode
		*parent = BTreeInnerNode();
		std::pair<BufferFrame*, BufferFrame*> result = split(bf_node, parent);
		bufferManager.unfixPage(bf_newRoot, true); //new rootNode can be unlocked since the two new children of it have enough space
		return result; //parent pointer in insert() will still be NULL which is correct
	}
	BTreeInnerNode* node = (BTreeInnerNode*) bf_node->getData();
	BufferFrame* bf_newNode = &bufferManager.fixPage(pageIdForPage(++maxPage), true); //unfix will be done in insert
	if(isLeaf(node)) {
		BTreeLeafNode* leaf = (BTreeLeafNode*) node;
		BTreeLeafNode* newLeaf = (BTreeLeafNode*) bf_newNode->getData();
		*newLeaf = BTreeLeafNode();
		//move about half the entries to the right
		memcpy(&newLeaf->entry[0], &leaf->entry[leaf->count/2], (leaf->count - leaf->count/2)*sizeof(BTreeKeyTIDPair));
		newLeaf->count = leaf->count - leaf->count/2;
		newLeaf->next = leaf->next;
		leaf->count = leaf->count/2;
		leaf->next = bf_newNode->pageId;
		//parent will have enough space for split key, because we split all full nodes along the way down
		insertKeyInInnerNode(biggestKey(leaf),bf_node->pageId, bf_newNode->pageId, parent);
	} else {
		BTreeInnerNode* innerNode = node;
		BTreeInnerNode* newInnerNode = (BTreeInnerNode*) bf_newNode->getData();
		*newInnerNode = BTreeInnerNode();
		//move about half the entries to the right
		memcpy(&newInnerNode->entry[0], &innerNode->entry[innerNode->count/2], (innerNode->count - innerNode->count/2)*sizeof(BTreeKeyTIDPair));
		newInnerNode->count = innerNode->count - innerNode->count/2;
		newInnerNode->upper = innerNode->upper;
		innerNode->upper = 0;
		innerNode->count = innerNode->count/2;
		insertKeyInInnerNode(biggestKey(innerNode),bf_node->pageId, bf_newNode->pageId, parent);
	}
	return std::make_pair(bf_node,bf_newNode);
};
//-----------------------------------------------------------------------------
template<typename K, typename Comp>
void BTree<K, Comp>::insertKeyInInnerNode(K key, PID leftPid, PID rightPid, BTreeInnerNode* node ) {
	uint64_t correctSlot = findKeyInNode(key, node);
	assert(node->count >= correctSlot);
	if(node->count == correctSlot) {
		node->upper = rightPid;
	}
	memmove(&node->entry[correctSlot+1], &node->entry[correctSlot],(node->count-correctSlot)*sizeof(BTreeKeyPIDPair));
	node->entry[correctSlot] = {key, leftPid};
	node->count++;

};
template<typename K, typename Comp>
bool BTree<K, Comp>::lookup (K key, TID& tid) {
	
	bool result = false;
	BufferFrame* bf_cur = &bufferManager.fixPage(rootNodePageId, false);
	BufferFrame* bf_par = NULL;
	BTreeInnerNode* currentNode = (BTreeInnerNode*) bf_cur->getData();
	BTreeInnerNode* parentNode = NULL;
	while(!isLeaf(currentNode)) {
		if(parentNode != NULL) {
			bufferManager.unfixPage(*bf_par, false);
		}
		bf_par = bf_cur;
		parentNode = currentNode;
		PID childPID = getChildNodePIDForKey(key, currentNode);
		bf_cur = &bufferManager.fixPage(childPID, false);
		currentNode = (BTreeInnerNode*) bf_cur->getData();
	}
	BTreeLeafNode* leaf = (BTreeLeafNode*) currentNode;
	uint64_t correctSlot = findKeyInNode(key, leaf);
	if(correctSlot == leaf->count) {
		//std::cout << "looking up out of range" << std::endl;
	}
	if(correctSlot < leaf->count && !smaller(key, leaf->entry[correctSlot].key) && !smaller(leaf->entry[correctSlot].key, key)) { //finKeyInNode can return first free slot as well
		
		tid = leaf->entry[correctSlot].tid;
		result = true;
	}
	if(bf_par != NULL) {
		bufferManager.unfixPage(*bf_par, false);
	}
	bufferManager.unfixPage(*bf_cur, false);
	return result;
};
template<typename K, typename Comp>
bool BTree<K, Comp>::erase(K key) {
	bool result = false;
	BufferFrame* bf_cur = &bufferManager.fixPage(rootNodePageId, true);
	BufferFrame* bf_par = NULL;
	BTreeInnerNode* currentNode = (BTreeInnerNode*) bf_cur->getData();
	BTreeInnerNode* parentNode = NULL;
	while(!isLeaf(currentNode)) {
		if(parentNode != NULL) {
			bufferManager.unfixPage(*bf_par, false);
		}
		bf_par = bf_cur;
		parentNode = currentNode;
		PID childPID = getChildNodePIDForKey(key, currentNode);
		bf_cur = &bufferManager.fixPage(childPID, true);
		currentNode = (BTreeInnerNode*) bf_cur->getData();
	}
	BTreeLeafNode* leaf = (BTreeLeafNode*) currentNode;
	uint64_t correctSlot = findKeyInNode(key, leaf);
	
	if(correctSlot < leaf->count && !smaller(key, leaf->entry[correctSlot].key) && !smaller(leaf->entry[correctSlot].key, key)) {
		leaf->count--;
		memmove(&leaf->entry[correctSlot], &leaf->entry[correctSlot+1],(leaf->count-correctSlot)*sizeof(BTreeKeyTIDPair));
		assert(leaf->count >=0);
		mSize--;
		result = true;
	}
	if(bf_par != NULL) {
		bufferManager.unfixPage(*bf_par, false);
	}
	bufferManager.unfixPage(*bf_cur, true);
	return result;
};