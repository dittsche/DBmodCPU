#ifndef SELECTION_OPERATOR_HPP
#define SELECTION_OPERATOR_HPP

class SelectionOperator : public Operator
{
	std::unique_ptr<Operator> input;
	std::vector<Register*> outputRegister;
	Register constantValue;
	unsigned registerId;
public:
	SelectionOperator(std::unique_ptr<Operator> input, unsigned registerId, Register constantValue)
	:
	input(std::move(input)),
	constantValue(constantValue),
	registerId(registerId)
	{};
	~SelectionOperator(){};
	

	void open() {
		input->open();
		outputRegister = this->input->getOutput();
	};
	bool next() {
		while(input->next()) {
			if(*outputRegister[registerId] == constantValue) {
				return true;
			}
		}
		return false;
	};
	void close() {
		input->close();
	};
	std::vector<Register*> getOutput() {
		return outputRegister;
	}
};

#endif