#include "attribute.hpp"

#include "unique_stack.hpp"
#include "coroutine.hpp"

co::Attribute::Attribute() :
	vaild_(true),
	stack_space_(new UniqueStack(StackSpace::default_stack_size)) {
}

co::Attribute::Attribute(StackSpace &&space) :
	vaild_(true),
	stack_space_(space.data()) {
}

co::Attribute::Attribute(Attribute &&attribute) :
	vaild_(attribute.vaild_),
	stack_space_(attribute.stack_space_) {
	attribute.vaild_ = false;
	stack_space_.reset();
}

void co::Attribute::init(Coroutine &coroutine) {
	coroutine.stack_space_ = stack_space_;
	if(!stack_space_->shared()) {
		stack_space_.reset();
		vaild_ = false;
	}
}