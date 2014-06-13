#ifndef PROJECTION_OPERATOR_HPP
#define PROJECTION_OPERATOR_HPP

class ProjectionOperator : public Operator
{
	std::unique_ptr<Operator> input;
	std::vector<Register*> outputRegister;
	std::vector<unsigned> projReg;
public:
	ProjectionOperator(std::unique_ptr<Operator> input, std::vector<unsigned>&& projReg)
	:
	input(std::move(input)),
	projReg(projReg)
	{
	};
	~ProjectionOperator(){};
	
	void open() {
		input->open();
		outputRegister.clear();

		std::vector<Register*> temp = input->getOutput();
		for(auto& pr : projReg) {
			outputRegister.push_back(temp[pr]);
		}
	};
	bool next() {
		return input->next();
	};
	void close() {
		input->close();
	};

	std::vector<Register*> getOutput() {
		return outputRegister;
	};
};


#endif