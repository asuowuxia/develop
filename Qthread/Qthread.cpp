
#include <iostream>
#include <thread>

template<typename T>
void show_list(T value)
{
    std::cout << value << std::endl;
}

// Args是一个模板参数包
// Args与若干个类型匹配（包括0）
template<typename... Args>
void shows_list(Args... args)
{
    // 那么函数如何访问这些包的内容呢？
    // C++11规定将省略号放在函数参数包名的右边可以将参数包展开，也就是说args…代表了函数的第一个参数


}

template<typename... Args>
void showss_list(Args... args)
{
    showss_list(args...);
}

template<class T,typename... Args>
void showsss_list(T value, Args... args)
{
    std::cout << value << std::endl;
    showsss_list(args...);
}

void show_listt()
{
    std::cout << "empty" << std::endl;
}

// 1.声明一个参数包T... args，这个参数包中可以包含0到任意个模板参数；
template<class T, class... Args>
void show_listt(T value, Args... args)
{
    cout << value << endl;
    // 2.省略号...在模板定义的右边，可以将参数包展开成一个一个独立的参数。
    show_list(args...);
}

// C++11的新特性--可变模版参数（variadic templates）是C++11新增的最强大的特性之一
// 可变模版参数函数和可变模版参数类

template <class... T>
void f(T... args)
{
    cout << sizeof...(args) << endl; //打印变参的个数
}


template<typename T>
T sum(T t)
{
    return t;
}
template<typename T, typename ... Args>
T sum(T first, Args... args)
{
    return first + sum<T>(args...);
}

// 递归函数展开参数包是一种标准做法，也比较好理解，但也有一个缺点,就是必须要一个重载的递归终止函数，即必须要有一个同名的终止函数来终止递归，这样可能会感觉稍有不便。
// 有没有一种更简单的方式呢？其实还有一种方法可以不通过递归方式来展开参数包，这种方式需要借助逗号表达式和初始化列表。比如前面print的例子可以改成这样：

class X
{
public:
    void do_lengthy_work();
    void do_lengthy_work1(int);
};

// 当原对象是一个临时变量时，自动进行移动操作，
// 但当原对象是一个命名变量，那么转移的时候就需要使用 std::move() 进行显示移动
void process_big_object(std::unique_ptr<big_object>);

std::thread f()
{
    void some_function();
    return std::thread(some_function);
}
std::thread g()
{
    void some_other_function(int);
    std::thread t(some_other_function, 42);
    return t;
}

void ff(std::thread t);
void gg()
{
    void some_function();
    f(std::thread(some_function));
    std::thread t(some_function);
    f(std::move(t));
}

class scoped_thread
{
    std::thread t;
public:
    explicit scoped_thread(std::thread t_);
    t(std::move(t_))
    {
        if (!t.joinable())
            throw std::logic_error("no thread");
    }
    ~scoped_thread()
    {
        t.join();
    }
    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

struct func;

void f()
{
    int some_local_state;
    scoped_thread t(std::thread(func(some_local_state)));
    do_something_in_current_thread();
}

int main()
{

    show_list(2.5);

    f();        //0
    f(1, 2);    //2
    f(1, 2.5, "");    //3

    sum(1, 2, 3, 4); //10

    X my_x;
    //std::thread构造的第三个参数为成员函数的第一个参数
    std::thread t(&X::do_lengthy_work, &my_x);
    int num(0);
    std::thread t(&X::do_lengthy_work1, &my_x, num);

    std::unique_ptr<big_object>p(new big_object);
    p->prepare_data(42);
    std::thread t(process_big_object, std::move(p));

    // std::terminate() 将调用std::thread的析构函数
    // 不能通过赋一个新值给 std::thread 对象的方式来"丢弃"一个线程)。


}
