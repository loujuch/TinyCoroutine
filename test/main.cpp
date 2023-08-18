#include <iostream>
#include <vector>

#include "coroutine.hpp"
#include "unique_stack.hpp"
#include "attribute.hpp"

int n = 0;

void func(int name, int loop) {
	for(int i = 0;i < loop;++i) {
		std::cout << name << ": " << i << std::endl;
		co::self().yield();
		++n;
	}
}

void test(int num, int loop) {
	std::vector<co::Coroutine> tmp(num);
	for(int i = 0;i < tmp.size();++i) {
		co::UniqueStack unique_stack;
		co::Attribute attribute(std::move(unique_stack));
		tmp[i] = std::move(co::Coroutine(attribute, func, i, loop));
	}
	int end = 0;
	while(end < tmp.size()) {
		end = 0;
		for(auto &p : tmp) {
			if(!p.can_resume()) {
				++end;
			} else {
				p.resume();
			}
		}
	}
	std::cout << n << " --- " << loop * num << std::endl;
}

int main(int argc, char *argv[]) {
	int num = 5;
	int loop = 5;
	if(argc > 1) {
		num = atoi(argv[1]);
		num = num > 0 ? num : 5;
	}
	if(argc > 2) {
		loop = atoi(argv[2]);
		loop = loop > 0 ? loop : 5;
	}
	for(int i = 0;i < 32;++i) {
		test(num, loop);
	}
	return 0;
}