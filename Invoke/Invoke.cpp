// Invoke.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <functional>

struct Foo
{
	Foo(int num) :_num(num) {}
	void print_add(int i) const
	{
		std::cout << _num + i << std::endl;
	}
	int _num;
};
void print_num(int i)
{
	std::cout << i << std::endl;
}

struct PrintNum
{
	void operator()(int i) const
	{
		std::cout << i << std::endl;
	}
};


int main()
{
	// 调用自由函数
	std::invoke(print_num, -9);
	// 调用 lambda
	std::invoke([]() { print_num(42); });
	// 调用成员函数
	const Foo foo(314159);
	std::invoke(&Foo::print_add, foo, 1);
	// 调用（访问）数据成员
	std::cout << "num_: " << std::invoke(&Foo::_num, foo) << std::endl;
	// 调用函数对象
	std::invoke(PrintNum(), 18);
#if 0
#endif
}
