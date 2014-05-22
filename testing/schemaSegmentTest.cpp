#include "../schema/Schema.hpp"
#include "../schemaParser/Parser.hpp"
#include "../segments/SchemaSegment.hpp"
#include "../buffer/BufferManager.hpp"
#include <iostream>
#include <string>

using namespace std;

int main() {
	unique_ptr<Parser> parser(new Parser("schemaParser/test.sql"));

	unique_ptr<Schema> schema_p = parser->parse();

	BufferManager b(1000);

	SchemaSegment* s = new SchemaSegment(b);
	int i = 1;
	for(auto& r: schema_p->relations) {
		r.segmentId = i++;
		r.size = 0;
	}
	s->serialize(*schema_p);
	//now persistent

	Schema s2 = s->deserialize();
	//cout << schema_p->toString() << endl << endl;
	cout << s2.toString();
	delete s;
}