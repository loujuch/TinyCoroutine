#ifndef _UNIQUE_STACK_HPP__
#define _UNIQUE_STACK_HPP__

#include <memory>

#include "stack_space.hpp"

namespace co {

class UniqueStack :public StackSpace {
	std::ptrdiff_t stack_capacity_;
	std::unique_ptr<char[]> stack_;
public:
	UniqueStack(std::ptrdiff_t stack_capacity = StackSpace::default_stack_size);
	UniqueStack(UniqueStack &&uniqueU_snitack);
	~UniqueStack();

	std::shared_ptr<co::StackSpace> data();

	inline std::ptrdiff_t capacity() const {
		return stack_capacity_;
	}

	inline char *top() const {
		return stack_.get();
	}

	inline void save() {
	}

	inline void load(Coroutine &coroutine) {
		owner_coroutine_ptr_ = &coroutine;
	}

	inline void destory(Coroutine &) {
		owner_coroutine_ptr_ = nullptr;
		stack_capacity_ = 0;
		stack_.reset();
	}
}; // class UniqueStack

} // namespace co

#endif // _UNIQUE_STACK_HPP__