#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <iostream>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

// Any 类型，可以接收任意数据的类型
class Any{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    // 这个模版构造函数，接收一个参数的返回值
    template <typename T>
    Any(T data):base_(std::make_unique<Derive<T>>(data)){}

    template <typename T>
    T cast_(){
        Derive<T>* pf = dynamic_cast<Derive<T>*>(base_.get());
        if(pf == nullptr)
        {
            throw "type is unmatch"
        }
        return pf->data_;
    }
private:

    // 定义基类类型,为了使用它的基类指针
    class Base{
    public:
        virtual ~Base() = default;
    };

    // 使用模版定义派生类,使用派生类将数据包住
    template<typename T>
    class Derive : public Base{
    public:
        Derive(T data):data_(date){}
        T data_; // 保存了任意的其他类型
    };

private:
    // 基类成员变量，基类指针指向派生类对象
    std::unique_ptr<Base>base_;
};

class Task{
public:
    virtual void run() = 0;
private:
};

// 线程池模式
enum class PoolMode{
    MODE_FIXED, // 固定线程数量的线程池模式
    MODE_CACHED // 动态增长的线程池模式
};


// 抽象线程模型
class Thread{
public:
    using ThreadFunc = std::function<void()>;

    // 构造函数
    Thread(ThreadFunc func);

    // 析构函数
    ~Thread();

    void start(); // 线程启动函数
private:
    ThreadFunc func_;
};

// 定义线程池类
class ThreadPool{
public:
    ThreadPool(); // 构造函数

    ~ThreadPool(); // 析构函数

    void start(int initThreadSize = 4); // 开启线程池

    void setMode(PoolMode mode); // 设置线程池类型

    void setMaxQueThreshold(int threshold); // 设置任务队列上限数量

    void submitTask(std::shared_ptr<Task>sp); // 任务提交接口

    // 禁止用户拷贝线程池
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void threadFunc(); // 定义线程执行函数

private:
    std::vector<std::unique_ptr<Thread>>threads_; // 线程存储列表
    size_t initThreadSize_; // 初始线程数量
    std::queue<std::shared_ptr<Task>>taskQueue_; // 存储任务的队列,使用智能指针存储
    int maxTaskQueThreshold_; // 任务数量阈值
    std::mutex taskQueMtx_; // 对任务队列操作需要保证线程安全，使用锁+条件变量
    std::atomic_int taskSize_; // 任务数量的加减需要保证线程安全，这里使用atomic原子操作来保证
    std::condition_variable notFull_; // 条件变量notfull表示任务队列不满
    std::condition_variable notEmpty_; // 条件变量notEmpty表示任务队列不空 
    PoolMode poolMode_; // 线程池运行类型
};
#endif