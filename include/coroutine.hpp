#ifndef _COROUTINE_HPP__
#define _COROUTINE_HPP__

#include <functional>
#include <memory>

#include "attribute.hpp"
#include "stack_space.hpp"

#include <ucontext.h>

namespace co {

// 协程类
class Coroutine {
public:
	friend void Attribute::init(Coroutine &coroutine);

	static constexpr std::ptrdiff_t STACK_SIZE = 1024 * 1024 * 32;

	enum class Status :uint8_t {
		COROUTINE_NONE = 0,
		COROUTINE_READY,
		COROUTINE_RUNNING,
		COROUTINE_SUSPEND,
		COROUTINE_DEAD,
		COROUTINE_MAIN
	};
private:
	using Functor = std::function<void()>;

	Functor functor_;
	Status status_;
	::ucontext_t context_;
	std::shared_ptr<StackSpace> stack_space_;

	void init(Attribute &&attribute);
	static void main_func(uint32_t low32, uint32_t hi32);

public:
	// 无参构造函数，制造空协程对象用于占位，空协程对象无法运行
	Coroutine();

	// 创建非空协程对象
	template <typename Callback_, typename ... Args_>
	explicit Coroutine(Callback_ callback, Args_ ... args) :
		functor_(std::bind(std::forward<Callback_>(callback), std::forward<Args_>(args)...)),
		status_(Status::COROUTINE_READY),
		stack_space_(nullptr) {
		init(Attribute());
	}

	// 创建非空协程对象
	template <typename Callback_, typename ... Args_>
	explicit Coroutine(Attribute &attribute, Callback_ callback, Args_ ... args) :
		functor_(std::bind(std::forward<Callback_>(callback), std::forward<Args_>(args)...)),
		status_(Status::COROUTINE_READY),
		stack_space_(nullptr) {
		init(std::move(attribute));
	}

	// 移动构造函数，入参变为空协程对象
	Coroutine(Coroutine &&coroutine);

	// 析构函数
	~Coroutine();

	// 移动函数，入参变为空协程对象
	Coroutine &operator=(Coroutine &&coroutine);

	// 恢复协程
	void resume();
	// 挂起协程
	void yield();
	// 退出协程
	void exit();

	// 获取协程状态
	inline Status status() const {
		return status_;
	}

	inline bool can_resume() const {
		return status_ != Status::COROUTINE_DEAD &&
			status_ != Status::COROUTINE_MAIN &&
			status_ != Status::COROUTINE_NONE;
	}

	Coroutine(const Coroutine &) = delete;
	Coroutine &operator=(const Coroutine &) = delete;
}; // class coroutine

// 返回当前上下文对应协程对象的引用，如果当前上下文没有对应的协程对象，则抛出 NoCoroutine 异常
Coroutine &self();

// 返回当前上下文是否在协程对象中
inline bool is_coroutine();

// 返回当前上下文是否为主协程（当前上下文不是协程对象返回false）
inline bool is_main_coroutine();

} // namespace co

#endif // _COROUTINE_HPP__