#include "Operator.hpp"
#include "../relation/Relation.hpp"

class TableScanOperator : public Operator
{
	unsigned current;
	Relation& relation;
	std::vector<Register> outputRegister;
	std::vector<Register*> outputRegisterPtr;

public:
	TableScanOperator(Relation& rel);
	~TableScanOperator();
	void open() {
		current = -1;
		outputRegister.clear();
		outputRegister.reserve(relation.numColumns());
		for(size_t i = 0; i < relation.numColumns(); i++) {
			outputRegister.push_back(Register());
			outputRegisterPtr.push_back(&outputRegister.back());
		}
	};
	bool next();
	void close() {
	};
	std::vector<Register*> getOutput() {
		return outputRegisterPtr;
	};
};

TableScanOperator::TableScanOperator(Relation& rel)
:
current(-1),
relation(rel)
{

};
TableScanOperator::~TableScanOperator() {

};
bool TableScanOperator::next() {
	current++;
	if(current < relation.size()) {
		std::vector<Register>& temp = relation.get(current);
		int i = 0;
		for(auto& cell : temp) {
			outputRegister[i++] = cell;
		}

		return true;
	}
	return false;
};
