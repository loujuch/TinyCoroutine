#ifndef _ATTRIBUTE_HPP__
#define _ATTRIBUTE_HPP__

#include "stack_space.hpp"

namespace co {

class Coroutine;

class Attribute {
	bool vaild_;
	std::shared_ptr<StackSpace> stack_space_;
public:
	Attribute();
	explicit Attribute(StackSpace &&space);
	Attribute(Attribute &&attribute);

	Attribute(const Attribute &) = delete;

	void init(Coroutine &coroutine);

	inline bool vaild() {
		return vaild_;
	}
}; // class Attribute

} // namespace co

#endif // _ATTRIBUTE_HPP__