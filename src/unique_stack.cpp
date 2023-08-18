#include "unique_stack.hpp"

co::UniqueStack::UniqueStack(std::ptrdiff_t stack_capacity) :
	StackSpace(false),
	stack_capacity_(stack_capacity),
	stack_(new char[stack_capacity_]) {
}

co::UniqueStack::UniqueStack(UniqueStack &&unique_stack) :
	StackSpace(false),
	stack_capacity_(unique_stack.stack_capacity_),
	stack_(std::move(unique_stack.stack_)) {
	unique_stack.stack_capacity_ = 0;
	unique_stack.stack_.reset();
}

co::UniqueStack::~UniqueStack() {
	owner_coroutine_ptr_ = nullptr;
	stack_capacity_ = 0;
	stack_.reset();
}

std::shared_ptr<co::StackSpace> co::UniqueStack::data() {
	return std::shared_ptr<co::StackSpace>(new UniqueStack(std::move(*this)));
}