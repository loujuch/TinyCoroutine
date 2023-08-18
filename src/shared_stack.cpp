#include "shared_stack.hpp"

#include "coroutine_exception.hpp"

#include <string.h>

co::SharedStack::SharedStack(std::ptrdiff_t stack_capacity) :
	StackSpace(true),
	stack_capacity_(stack_capacity),
	stack_(new char[stack_capacity_]),
	backup_stack_{} {
}

co::SharedStack::SharedStack(SharedStack &&shared_stack) :
	StackSpace(true),
	stack_capacity_(shared_stack.stack_capacity_),
	stack_(std::move(shared_stack.stack_)),
	backup_stack_(std::move(shared_stack.backup_stack_)) {
	shared_stack.stack_capacity_ = 0;
	shared_stack.stack_.reset();
	shared_stack.backup_stack_.clear();
}

co::SharedStack::~SharedStack() {
	stack_capacity_ = 0;
	stack_.reset();
}

std::shared_ptr<co::StackSpace> co::SharedStack::data() {
	return std::shared_ptr<co::StackSpace>(new SharedStack(std::move(*this)));
}

void co::SharedStack::save() {
	if(owner_coroutine_ptr_ == nullptr) {
		return;
	}
	char dummy = 0;
	if(stack_.get() - &dummy > stack_capacity_) {
		throw CoroutineException{ "Coroutine stack overflow!" };
	}
	std::ptrdiff_t size = stack_.get() - &dummy;
	char *data_ptr = new char[size];
	memcpy(data_ptr, &dummy, size);
	auto p = backup_stack_.find(owner_coroutine_ptr_);
	if(backup_stack_.end() == p) {
		backup_stack_.emplace(owner_coroutine_ptr_, std::move(std::pair(size, data_ptr)));
	} else {
		p->second = std::move(std::pair(size, data_ptr));
	}
}

void co::SharedStack::load(Coroutine &coroutine) {
	if(&coroutine == owner_coroutine_ptr_) {
		return;
	}
	auto p = backup_stack_.find(&coroutine);
	if(backup_stack_.end() == p) {
		throw;
	}
}

void co::SharedStack::destory(Coroutine &coroutine) {
	auto p = backup_stack_.find(&coroutine);
	if(p == backup_stack_.end()) {
		return;
	}
	backup_stack_.erase(p);
}