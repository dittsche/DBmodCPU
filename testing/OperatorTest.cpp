
#include <iostream>
#include <memory>
#include <vector>
#include "../operators/TableScanOperator.hpp"
#include "../operators/PrintOperator.hpp"
#include "../operators/ProjectionOperator.hpp"
#include "../operators/SelectionOperator.hpp"
#include "../operators/HashJoinOperator.hpp"
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
	hoeren.insert({Register(4711),Register(42)});
	hoeren.insert({Register(4713),Register(23)});
	hoeren.insert({Register(42),Register("asd")});

	Relation studenten;
	studenten.addAttribute("matrnr", Relation::Attribute::Type::Int);
	studenten.addAttribute("name", Relation::Attribute::Type::String);
	studenten.addAttribute("sws", Relation::Attribute::Type::Int);

	studenten.insert({Register(12),Register("Fichte"), Register(4)});
	studenten.insert({Register(4711),Register("Carnas"), Register(6)});
	studenten.insert({Register(4711),Register("Carnas2"), Register(3)});
	studenten.insert({Register(4711),Register("Carnas3"), Register(6)});

	unique_ptr<Operator> hoeren_scan(new TableScanOperator(hoeren));
	unique_ptr<Operator> studenten_scan(new TableScanOperator(studenten));

	unique_ptr<Operator> studenten_selection(new SelectionOperator(move(studenten_scan), 2, Register(6)));

	unique_ptr<Operator> hashJoin(new HashJoinOperator(move(studenten_selection), move(hoeren_scan), 0,0));
	unique_ptr<Operator> projection(new ProjectionOperator(move(hashJoin), {0,1,2,4}));

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