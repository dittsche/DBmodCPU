#include "Schema.hpp"

#include <sstream>

static std::string type(const Schema::Relation::Attribute& attr) {
   Types::Tag type = attr.type;
   switch(type) {
      case Types::Tag::Integer:
         return "Integer 0 ";
      /*case Types::Tag::Numeric: {
         std::stringstream ss;
         ss << "Numeric(" << attr.len1 << ", " << attr.len2 << ")";
         return ss.str();
      }*/
      case Types::Tag::Char: {
         std::stringstream ss;
         ss << "Char " << attr.len << " ";
         return ss.str();
      }
   }
   throw;
}

std::string Schema::toString() const {
   std::stringstream out;
   out << relations.size() << std::endl;
   for (const Schema::Relation& rel : relations) {
      out << rel.name << " " << rel.segmentId << " " << rel.size << " " << rel.attributes.size() << " " << rel.primaryKey.size() << std::endl;
      for (unsigned keyId : rel.primaryKey)
         out << ' ' << keyId;
      out << std::endl;
      for (const auto& attr : rel.attributes)
         out << attr.name << ' ' << type(attr) << (attr.notNull ? "1" : "0") << std::endl;
   }
   return out.str();
}
