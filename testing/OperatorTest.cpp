
#include <iostream>
#include <memory>
#include <vector>
#include "../operators/TableScanOperator.hpp"
#include "../operators/PrintOperator.hpp"
#include "../operators/ProjectionOperator.hpp"
#include "../operators/SelectionOperator.hpp"
#include "../relation/Relation.hpp"

using namespace std;

int main()
{
	Relation hoeren;
	hoeren.addAttribute("matrnr", Relation::Attribute::Type::Int);
	hoeren.addAttribute("vorlnr", Relation::Attribute::Type::Int);

	hoeren.insert({Register(12),Register(23)});
	hoeren.insert({Register(4711),Register(7)});
	hoeren.insert({Register(4711),Register(13)});
	hoeren.insert({Register(4713),Register(23)});
	hoeren.insert({Register(42),Register("asd")});


	unique_ptr<Operator> scan(new TableScanOperator(hoeren));

	unique_ptr<Operator> selection(new SelectionOperator(move(scan), 0, Register(4711)));
	unique_ptr<Operator> projection(new ProjectionOperator(move(selection), {0,1}));

	unique_ptr<Operator> print(new PrintOperator(move(projection),cout));
	print->open();

	while(print->next()) {}
	print->close();


	cout << "Register(42) == Register(43): " << (Register(42) == Register(43)) << endl;
	cout << "Register(42) == Register(42): " << (Register(42) == Register(42)) << endl;
	cout << "Register(42) < Register(43): " << (Register(42) < Register(43)) << endl;
	cout << "Register(43) < Register(42): " << (Register(43) < Register(42)) << endl;
	return 0;
}