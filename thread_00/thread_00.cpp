// thread_00.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <mutex>

void foo()
{
    std::cout << "hello world" << std::endl;
}

void some_operation(const std::string& message)
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    /// ... 操作
    // 当离开这个作用域的时候，互斥锁会被析构，同时unlock互斥锁
    // 因此这个函数内部的可以认为是临界区
}

std::mutex mtx;

void block_area()
{
    std::unique_lock<std::mutex> lock(mtx);
    // ...临界区
}

#include <future>

int main()
{
    // get_id() 获取所创建的线程的线程id
    // 使用join来加入一个线程
    std::thread t(foo);
    t.join();

    std::thread thd1(block_area);
    thd1.join();

    // std::future 提供了一个访问异步操作结果的途径
    // 可以理解为一种简单的线程同步手段，等待异步的线程B的结果，然后再开始A之后的工作
    // 
    // std::packaged_task 可以用来封装任何可以调用的目标，从而实现异步的调用
    // 1、function obj 2、function member 3、normal function 4、lamda 5、function pointer
    // 将一个返回值为7的lamda表达式封装到task中
    // std::packaged_task 的模板参数为要封装函数的类型
    std::packaged_task<int()> task([]() {return 7; });
    // 获得 task的future 在一个线程中执行task
    std::future<int> result = task.get_future();
    std::thread(std::move(task)).detach();


    return 0;


}

