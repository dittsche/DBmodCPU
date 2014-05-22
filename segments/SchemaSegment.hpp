#ifndef SCHEMA_SEGMENT_H
#define SCHEMA_SEGMENT_H

#include "../schema/Schema.hpp"
#include "../buffer/BufferManager.hpp"
#include "../buffer/BufferFrame.hpp"
#include <memory>


class SchemaSegment {
	public:
		SchemaSegment(BufferManager& b);
		void serialize(Schema& s);
		Schema deserialize();
	private:
		BufferManager& bufferManager;
};


#endif