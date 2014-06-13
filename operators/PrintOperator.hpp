#ifndef PRINT_OPERATOR_HPP
#define PRINT_OPERATOR_HPP

#include "Operator.hpp"

class PrintOperator : public Operator
{
	std::ostream& outstream;
	std::unique_ptr<Operator> input;
public:
	PrintOperator(std::unique_ptr<Operator> input, std::ostream& ostr)
	:
		outstream(ostr),
		input(std::move(input))
	{
	};
	~PrintOperator(){};
	void open() {
		input->open();
	};
	bool next() {

		if(input->next()) {
			std::vector<Register*> out = input->getOutput();
			for(auto& i : out) {
				outstream << *i;
			}
			outstream << std::endl;
			return true;
		} else {
			return false;
		}
	};
	void close() {
		input->close();
	};
	std::vector<Register*> getOutput() {
		return input->getOutput();
	}
};
#endif