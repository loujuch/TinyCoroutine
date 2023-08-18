#ifndef _COROUTINE_EXCEPTION_HPP__
#define _COROUTINE_EXCEPTION_HPP__

#include <exception>

namespace co {

class CoroutineException :public std::exception {
	const char *what_str_;
public:
	explicit CoroutineException(const char *what_str = "Coroutine Exception!") :
		what_str_(what_str) {

	}

	inline const char *what() const throw() {
		return what_str_;
	}
}; // class NoCoroutine

} // namespace co

#endif // _COROUTINE_EXCEPTION_HPP__