#ifndef RELATION_HPP
#define RELATION_HPP

class Relation
{
public:
	struct Attribute {
		enum class Type : unsigned {Int, String};
		std::string name;
		Type type;
	};
private:
	std::vector<Attribute> attributes;
	std::vector<std::vector<Register>> data;
	size_t columns;
public:
	Relation()
	:
	columns(0)
	{};
	~Relation() {};
	void addAttribute(std::string name, Attribute::Type type) {
		attributes.push_back({name, type});
		columns++;
	};
	void insert(std::vector<Register>&& tuple) {
		data.push_back(tuple);
	};
	std::vector<Register>& get(unsigned idx) {
		return data[idx];
	};
	size_t size() {
		return data.size();
	};
	size_t numColumns() {
		return columns;
	};
};

#endif