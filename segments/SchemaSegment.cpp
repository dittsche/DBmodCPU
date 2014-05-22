#include "SchemaSegment.hpp"

#include <iostream>
#include <sstream>

using namespace std;


SchemaSegment::SchemaSegment(BufferManager& b)
:
	bufferManager(b)
{

};

void SchemaSegment::serialize(Schema& schema) {

	BufferFrame& bf = bufferManager.fixPage(0,true);
	char* data = (char*)bf.getData();
	string schemaString = schema.toString();
	size_t len = schemaString.copy(data, schemaString.length());
	data[len] = '\0';
	//cout << data;
	bufferManager.unfixPage(bf, true);
};
Schema SchemaSegment::deserialize() {

	Schema schema;
	BufferFrame& bf = bufferManager.fixPage(0,false);
	char* data = (char*)bf.getData();
	stringstream schemastream(data);
	string relationName, attributeName, attributeType;
	uint64_t relationCount, size, segmentId, attributeCount, primaryKeyCount, attributeLength,primaryKeyIdx;
	bool notNull;
	schemastream >> relationCount;
	schema.relations.clear();
	while(relationCount-- > 0) {
		schemastream >> relationName;
		schemastream >> segmentId;
		schemastream >> size;
		schemastream >> attributeCount;
		schemastream >> primaryKeyCount;
		schema.relations.push_back(Schema::Relation(relationName));
		schema.relations.back().size = size;
		schema.relations.back().segmentId = segmentId;
		while(primaryKeyCount-- > 0) {
			schemastream >> primaryKeyIdx;
			schema.relations.back().primaryKey.push_back(primaryKeyIdx);
		}
		while(attributeCount-- > 0) {
			schemastream >> attributeName;
			schemastream >> attributeType;
			schemastream >> attributeLength;
			schemastream >> notNull;
			schema.relations.back().attributes.push_back(Schema::Relation::Attribute());
			schema.relations.back().attributes.back().name = attributeName;
			schema.relations.back().attributes.back().type = (attributeType == "Integer")?(Types::Tag::Integer):(Types::Tag::Char);
			schema.relations.back().attributes.back().len = attributeLength;
		}
	}
	bufferManager.unfixPage(bf, false);
	return schema;
};