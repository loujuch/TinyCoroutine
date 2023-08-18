#ifndef _STACK_SPACE_HPP__
#define _STACK_SPACE_HPP__

#include <memory>

#include <stddef.h>

namespace co {

class Coroutine;

// 协程的栈空间
class StackSpace {
public:
	static constexpr std::ptrdiff_t default_stack_size = 32 * 1024 * 1024;
private:
	bool shared_;
protected:
	const Coroutine *owner_coroutine_ptr_;
public:
	explicit StackSpace(bool shared) :
		shared_(shared),
		owner_coroutine_ptr_(nullptr) {
	}

	// 返回栈顶
	virtual char *top() const = 0;
	// 返回栈容量
	virtual std::ptrdiff_t capacity() const = 0;
	// 将数据转移到堆中，原对象变为无效对象。
	virtual std::shared_ptr<co::StackSpace> data() = 0;
	// 保存数据，通常用于共享栈
	virtual void save() = 0;
	// 恢复数据，通常用于共享栈
	virtual void load(Coroutine &) = 0;
	// 删除数据，通常用于共享栈
	virtual void destory(Coroutine &) = 0;

	// 判断调用目标协程是否会导致栈冲突
	inline bool conflict(const Coroutine &coroutine) const {
		return owner_coroutine_ptr_ != &coroutine;
	}

	inline bool shared() const {
		return shared_;
	}

	virtual ~StackSpace() = default;
}; // class StackSpace

} // namespace co

#endif // _STACK_SPACE_HPP__