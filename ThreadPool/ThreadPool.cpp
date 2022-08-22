// ThreadPool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <queue>
#include <future>

template<typename T>
class thread_safe_queue
{
public:
    // constructor
    thread_safe_queue() {}
    thread_safe_queue(const thread_safe_queue& other)
    {
        std::lock_guard<std::mutex> lock{ other.mutex };
        queue = other.queue;
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock{ mutex };
        queue.push(new_value);
        // 唤醒一个等待当前std::condition_varaiable的实例的线程
        // 唤醒一个等待(*this)的线程
        // std::condition_variable中的notify_one(),nofity_all wait wait_for wait_util串行调用
        cond.notify_one();
        // std::condition_variable::nofity_all()
        // 唤醒所有等待当前std::condition_variable的实例的线程
        // std::condition_variable::wait 
        //自动解锁lock对象，对于线程等待线程，当其他线程调用notify_one()或notify_all()时被唤醒，
        //    亦或该线程处于伪唤醒状态。在wait()返回前，lock对象将会再次上锁。
        // void wait(std::unique_lock<std::mutex>& lock);
        //等待 std::condition_variable 上的notify_one()或notify_all()被调用，或谓词为true的情况，来
        //    唤醒线程。
  /*      template<typename Predicate>
        void wait(std::unique_lock<std::mutex>&lock, Predicate pred);*/
        //效果
        //    正如
        //    while (!pred())
        //    {
        //        wait(lock);
        //    }
        //std::condition_variable 在调用notify_one()、调用notify_all()、超时或线程伪唤醒时，结束
        //    等待。
        //template<typename Rep, typename Period>
        //cv_status wait_for(
        //    std::unique_lock<std::mutex>&lock,
        //    std::chrono::duration<Rep, Period> const& relative_time);
  /*      当其他线程调用notify_one()或notify_all()函数时，或超出了relative_time的时间，亦或是线程
            被伪唤醒，则将lock对象自动解锁，并将阻塞线程唤醒。当wait_for()调用返回前，lock对象会
            再次上锁。*/
        //std::condition_variable::wait_for 需要一个谓词的成员函数重
        //    载
        //std::condition_variable 在调用notify_one()、调用notify_all()、超时或线程伪唤醒时，结束
        //    等待。
        //    template<typename Rep, typename Period, typename Predicate>
        //bool wait_for(
        //    std::unique_lock<std::mutex>&lock,
        //    std::chrono::duration<Rep, Period> const& relative_time,
        //    Predicate pred);

    }
    T wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lock{ mutex };
        cond.wait(lock, [this] { return !queque.empty(); });
        value = queue.front();
        queue.pop();
        return value;
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lock{ mutex };
        cond.wait(lock, [this] { return !queue.empty();  });
        if (queue.empty())
        {
            return false;
        }
        value = queue.front();
        queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lock{ mutex };
        if (queue.empty())
            return std::shared_ptr<T>{};
        std::shared_ptr<std::mutex> res{ std::make_shared<T>(queue.front()) };
        queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock{ mutex };
        return queue.empty();
    }
private:
    mutable std::mutex mutex;
    std::condition_variable cond;
    std::queue<T> queue;
};
class join_threads
{
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_):
        threads(threads_){}
    ~join_threads() {
        for (unsigned long i = 0; i < threads.size(); ++i)
        {
            if (threads[i].joinable())
                threads[i].join();
        }
    }
};

class thread_pool
{
    std::atomic_bool done;
    thread_safe_queue<std::function<void()> > work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;

    // 每个线程池里的线程可以玩耍的地方
    // 在自己权限许可的地方找个可以合适的工作来开始工作，也就是把线程池里的事务取出来工作
    void worker_thread()
    {
        while (!done)
        {
            std::function<void()> task;

            if (work_queue.wait_and_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
public:
    thread_pool() :
        done(false), joiner(threads)
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            // 给线程池里的每个线程分配一个可供每个线程玩耍的代码区就是work_thread
            for (unsigned i = 0; i < thread_count; ++i)
            {
                threads.push_back(std::thread(&thread_pool::worker_thread, this));
            }
        }
        catch (...)
        {
            done = true;
            throw;
        }
    }
    ~thread_pool()
    {
        done = true;
    }
    template<typename FunctionType>
    //  这个函数写得真鸡巴漂亮，因为把函数对象装到线程安全的队列中去，这不就是消息队列吗，而且还是安全的消息队列
    // 生成一个指名道姓的漂亮的std::function函数不香吗？
    // 这个是把“事务”提交到消息队列中去
    void submit(FunctionType f)
    {
        work_queue::push(std::function<void()>(f));
    }
};
class function_wrapper
{
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base(){ }
    };
    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type : impl_base
    {
        F f;
        empl_type(F&& f_): f(std::move(f_)){}
        void call() { f(); }
    };
public:
    template<typename F>
    function_wrapper(F&& f) :
        impl(new impl_type<F>(std::move(f)))
    {}
    void operator()() { impl->call(); }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) :
        impl(std::move(other.impl))
    {}
    function_wrapper& operator=(function_wrapper&& other)
    {
        impl = std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};

class thread_pool
{
    thread_safe_queue<function_wrapper> work_queue; // 使用function_wrapper，而非使用std::function
    void worker_thread()
    {
        while (!done)
        {
            function_wrapper task;
            if (work_queue.try_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
public:
    template<typename FunctionType>
    // 要知道提供函数返回类型所以使用std::result_of FunctionType引用实例
    // ::type
    std::future<typename std::result_of<FunctionType()>::type> // 1
        submit(FunctionType f)
    {
        // 表明类型 typename std::result_of<FunctionType()>::type
        // 对类型使用typedef 就是一绝
        //using result_type = typename std::result_of<FunctionType()>::type
        typedef typename std::result_of<FunctionType()>::type
            result_type; // 2
        std::packaged_task<result_type()> task(std::move(f)); // 3
        std::future<result_type> res(task.get_future()); // 4
        work_queue.push(std::move(task)); // 5
        return res; // 6
    }
    // 休息一下
};


int main()
{
    

}
