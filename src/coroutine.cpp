#include "coroutine.hpp"

#include "coroutine_exception.hpp"

#include <assert.h>
#include <string.h>

namespace co {
__thread std::size_t num = 0;
__thread Coroutine *main_coroutine = nullptr;
__thread Coroutine *now_coroutine = nullptr;
} // namespace co

void co::Coroutine::main_func(uint32_t low32, uint32_t hi32) {
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	co::Coroutine *coroutine = (co::Coroutine *)ptr;
	coroutine->functor_();
	coroutine->status_ = Status::COROUTINE_DEAD;
	now_coroutine = main_coroutine;
}

void co::Coroutine::init(Attribute &&attribute) {
	if(main_coroutine == nullptr) {
		// 创建第一个非空协程后自动建立协程环境
		main_coroutine = new Coroutine();
		main_coroutine->status_ = Status::COROUTINE_MAIN;
		now_coroutine = main_coroutine;
	}
	if(attribute.vaild()) {
		attribute.init(*this);
	} else {
		throw CoroutineException{ "invaild attribute!" };
	}
	++num;
}

co::Coroutine::Coroutine() :
	functor_(),
	stack_space_(nullptr),
	status_(Status::COROUTINE_NONE) {
	memset(&context_, 0, sizeof(context_));
	++num;
}

co::Coroutine::Coroutine(Coroutine &&coroutine) :
	functor_(std::move(coroutine.functor_)),
	status_(coroutine.status_),
	context_(coroutine.context_),
	stack_space_(std::move(coroutine.stack_space_)) {
	++num;
	// 将输入参数变为空协程
	coroutine.functor_ = Functor();
	coroutine.stack_space_.reset();
	coroutine.status_ = Status::COROUTINE_NONE;
	memset(&coroutine.context_, 0, sizeof(coroutine.context_));
	// TODO: 当前上下文为被输入协程
}

co::Coroutine::~Coroutine() {
	// 设置协程对象为死协程
	functor_ = Functor();
	status_ = Status::COROUTINE_DEAD;
	stack_space_.reset();
	memset(&context_, 0, sizeof(context_));
	--num;
	if(num == 1) {
		delete now_coroutine;
		main_coroutine = nullptr;
		now_coroutine = nullptr;
	}
	// TODO: 当前上下文为被析构协程
}

co::Coroutine &co::Coroutine::operator=(Coroutine &&coroutine) {
	// 设置目标协程
	functor_ = std::move(coroutine.functor_);
	status_ = coroutine.status_;
	stack_space_ = std::move(coroutine.stack_space_);
	memcpy(&context_, &coroutine.context_, sizeof(context_));

	// 将输入参数变为空协程
	coroutine.functor_ = Functor();
	coroutine.status_ = Status::COROUTINE_NONE;
	coroutine.stack_space_.reset();
	memset(&coroutine.context_, 0, sizeof(coroutine.context_));
	// TODO: 当前上下文为被析构协程

	return *this;
}

void co::Coroutine::resume() {
	// 只能在主协程中调度其他协程
	assert(is_main_coroutine());

	// 便于移动指针
	uintptr_t ptr = (uintptr_t)this;
	switch(status_) {
		// 第一次启动该线程
		case Status::COROUTINE_READY:
			// 初始化协程上下文
			getcontext(&context_);
			context_.uc_stack.ss_sp = stack_space_->top();
			context_.uc_stack.ss_size = stack_space_->capacity();
			context_.uc_link = &main_coroutine->context_;

			// 设置执行函数
			makecontext(&context_, (void (*)(void)) main_func,
				2, (uint32_t)ptr, (uint32_t)(ptr >> 32));

			// 修改协程状态
			status_ = Status::COROUTINE_RUNNING;
			now_coroutine = this;

			// 如果对应的栈为共享栈，且当前栈的使用者不为目标协程
			// TODO: 调用协程为当前栈运行协程
			if(stack_space_->conflict(*this)) {
				stack_space_->save();
				stack_space_->load(*this);
			}

			// 进行调度
			swapcontext(&main_coroutine->context_, &context_);
			break;
		case Status::COROUTINE_SUSPEND:
			// 修改协程状态
			status_ = Status::COROUTINE_RUNNING;
			now_coroutine = this;

			// 如果对应的栈为共享栈，且当前栈的使用者不为目标协程
			// TODO: 调用协程为当前栈运行协程
			if(stack_space_->conflict(*this)) {
				stack_space_->save();
				stack_space_->load(*this);
			}

			// 进行调度
			swapcontext(&main_coroutine->context_, &context_);
			break;
		default:
			break;
	}
}

void co::Coroutine::yield() {
	// 不能退出主协程
	assert(co::now_coroutine == nullptr ||
		co::Coroutine::Status::COROUTINE_MAIN != co::now_coroutine->status());

	// 不能退出未运行协程
	if(now_coroutine != this) {
		return;
	}

	// 修改协程状态
	status_ = Status::COROUTINE_SUSPEND;
	now_coroutine = main_coroutine;

	swapcontext(&context_, &main_coroutine->context_);
}

void co::Coroutine::exit() {
	assert(is_coroutine() &&
		co::Coroutine::Status::COROUTINE_MAIN == co::now_coroutine->status());

}

co::Coroutine &co::self() {
	if(co::now_coroutine == nullptr) {
		throw co::CoroutineException{ "Not Coroutine Environment!" };
	}
	return *co::now_coroutine;
}

bool co::is_coroutine() {
	return co::now_coroutine != nullptr;
}

bool co::is_main_coroutine() {
	return co::now_coroutine != nullptr &&
		co::Coroutine::Status::COROUTINE_MAIN == co::now_coroutine->status();
}