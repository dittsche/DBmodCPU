#ifndef HASHJOIN_OPERATOR_HPP
#define HASHJOIN_OPERATOR_HPP

#include "Operator.hpp"
#include <unordered_map>
#include <vector>
#include <iostream>

class HashJoinOperator : public Operator
{
	std::unique_ptr<Operator> leftInput;
	std::unique_ptr<Operator> rightInput;
	std::vector<Register*> outputRegister;
	std::vector<Register> leftRegister;
	Register* rightRegisterPtr;
	unsigned leftRegisterId;
	unsigned rightRegisterId;
	typedef std::unordered_multimap<Register, std::vector<Register>> hashTable_t;
	hashTable_t hashTable;
	std::pair<hashTable_t::iterator,hashTable_t::iterator> range;
	bool getNewTupleFromRight;

public:
	HashJoinOperator(std::unique_ptr<Operator> left, std::unique_ptr<Operator> right, unsigned leftRegisterId, unsigned rightRegisterId)
	:
	leftInput(std::move(left)),
	rightInput(std::move(right)),
	leftRegisterId(leftRegisterId),
	rightRegisterId(rightRegisterId)
	{};
	~HashJoinOperator(){};
	
	void open() {
		leftInput->open();
		rightInput->open();

		auto leftInputRegister = leftInput->getOutput();
		auto rightRegister = rightInput->getOutput();
		rightRegisterPtr = rightRegister[rightRegisterId];
		leftRegister.resize(leftInputRegister.size());
		for(auto& r: leftRegister) {
			outputRegister.push_back(&r);
		}
		for(auto& r_p: rightRegister) {
			outputRegister.push_back(r_p);
		}

		Register* leftRegisterPtr = leftInputRegister[leftRegisterId];
		while (leftInput->next()) {
			std::vector<Register> temp;
			for(auto& r : leftInputRegister) {
				temp.push_back(*r);
			}
			hashTable.insert(std::make_pair(*leftRegisterPtr, temp));
		}
		leftInput->close();

		getNewTupleFromRight = true;

	};

	bool next() {
		while(true) {
			if(getNewTupleFromRight) {

				if(!rightInput->next()) {
					return false;
				}
				getNewTupleFromRight = false;

				range = hashTable.equal_range(*rightRegisterPtr);
			}

			if(range.first != range.second) {
				std::vector<Register>& leftSide = range.first->second;
				for(size_t i = 0;i < leftSide.size(); i++) {
					leftRegister[i] = leftSide[i];
				}
				range.first++;
				return true;
			} else {
				getNewTupleFromRight = true;
			}
		}
	};

	void close() {
		rightInput->close();
	};

	std::vector<Register*> getOutput() {
		return outputRegister;
	};
};

#endif