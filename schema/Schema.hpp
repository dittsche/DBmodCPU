#ifndef H_Schema_hpp
#define H_Schema_hpp

#include <vector>
#include <string>
#include "Types.hpp"

struct Schema {
   struct Relation {
      struct Attribute {
         std::string name;
         Types::Tag type;
         unsigned len;
         bool notNull;
         Attribute() : len(~0), notNull(true) {}
      };
      std::string name;
      std::vector<Schema::Relation::Attribute> attributes;
      std::vector<unsigned> primaryKey;
      uint64_t segmentId;
      uint64_t size;
      Relation(const std::string& name) : name(name), segmentId(1), size(0) {}
   };
   std::vector<Schema::Relation> relations;
   std::string toString() const;
};
#endif
