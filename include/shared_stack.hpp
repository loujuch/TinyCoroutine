#ifndef _SHARED_STACK_HPP__
#define _SHARED_STACK_HPP__

#include <map>

#include "stack_space.hpp"

namespace co {

class SharedStack :public StackSpace {
	std::ptrdiff_t stack_capacity_;
	std::unique_ptr<char[]> stack_;

	// 临时存储用
	std::map<const Coroutine *,
		std::pair<std::ptrdiff_t, std::unique_ptr<char[]>>> backup_stack_;
public:
	explicit SharedStack(std::ptrdiff_t stack_capacity = StackSpace::default_stack_size);
	SharedStack(SharedStack &&shared_stack);
	~SharedStack();

	std::shared_ptr<co::StackSpace> data();
	void save();
	void load(Coroutine &coroutine);
	void destory(Coroutine &coroutine);

	inline std::ptrdiff_t capacity() const {
		return stack_capacity_;
	}

	inline char *top() const {
		return stack_.get();
	}
}; // class SharedStack

} // namespace co

#endif // _SHARED_STACK_HPP__