#ifndef REGISTER_HPP
#define REGISTER_HPP

#include "stdint.h"
#include <string>
#include <exception>


class Register
{
public:
	enum class State {Undefined, Int, String};
private:
	int integerValue;
	std::string stringValue;
	State state;
public:
	Register()
	:
		state(State::Undefined)
	{};
	~Register() {};
	
	Register(int val) {
		setInteger(val);
	};
	Register(const std::string& s) {
		setString(s);
	};

	State getState() {
		return state;
	}
	int getInteger() const {
		return integerValue;
	};
	void setInteger(const int val) {
		integerValue = val;
		state = State::Int;
	};
	void setString(const std::string& s) {
		stringValue = s;
		state = State::String;
	}
	std::string getString() const {
		return stringValue;
	};

	size_t hash() const {
		std::hash<int> int_hash;
		std::hash<std::string> string_hash;
		switch(state) {
			case State::Int:
				return int_hash(integerValue);
			case State::String:
				return string_hash(stringValue);
			default:
				throw std::exception();
		}
	};

	bool operator==(Register rhs) const {
		if(this->state != rhs.state) return false;

		switch(this->state) {
			case State::Int:
				if(this->getInteger() == rhs.getInteger()) {
					return true;
				}
				break;
			case State::String:
				if(this->getString() == rhs.getString()) {
					return true;
				}
				break;
			default: return false;
		}
		return false;
	};
	bool operator<(Register rhs) {
		if(this->state != rhs.state) {
			throw std::exception();
		}
		switch(this->state) {
			case State::Int:
				if(this->getInteger() < rhs.getInteger()) {
					return true;
				}
				break;
			case State::String:
				if(this->getString() < rhs.getString()) {
					return true;
				}
				break;
			default: return false;
		}
		return false;
	};
};

namespace std {

  template <>
  struct hash<Register>
  {
    std::size_t operator()(const Register& k) const
    {
      return k.hash();
    }
  };
}

std::ostream& operator<<(std::ostream& lhs, Register& rhs) {
	switch(rhs.getState()) {
		case Register::State::Int:
			lhs << rhs.getInteger();
			break;
		case Register::State::String:
			lhs << rhs.getString();
			break;
		default:
			lhs << "NULL";
	}
	lhs << " ";
	return lhs;
}

#endif