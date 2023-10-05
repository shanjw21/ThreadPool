#include "threadPool.h"
#include <functional>
#include <thread>
const int maxTaskQueThreshold = 4;

// 定义线程池构造函数
ThreadPool::ThreadPool()
    :initThreadSize_(0)
    ,maxTaskQueThreshold_(maxTaskQueThreshold)
    ,taskSize_(0)
    ,poolMode_(PoolMode::MODE_FIXED)
{

}

// 定义线程池析构函数
ThreadPool::~ThreadPool()
{

}

void ThreadPool::setMode(PoolMode mode) // 设置线程池类型
{
    poolMode_ = mode;
}

void ThreadPool::setMaxQueThreshold(int threshold) // 设置任务队列上限数量
{
    maxTaskQueThreshold_ = threshold;
}

// 生产者模型，用户通过此接口，向任务队列中添加任务。
void ThreadPool::submitTask(std::shared_ptr<Task>sp) // 任务提交接口
{
    // 1. 获取锁,将任务队列加锁
    std::unique_lock<std::mutex>lock(taskQueMtx_);

    // 2. notFull_线程通信，等待任务队列有空余, 如果任务队列中任务满了则等待,再次阻塞
    // 用户提交任务由于任务队列满导致提交失败，不能一直阻塞，如果1s还未成功则提示任务提交失败
    if(!notFull_.wait_for(lock,std::chrono::seconds(1),
    [&]()->bool{return taskQueue_.size() < (size_t)maxTaskQueThreshold_;}))
    {
        // 如果 wait_for 返回值为false,说明1s时间内任务提交失败
        std::cerr << "task queue is full , task submit fail." << std::endl;
        return;
    }

    // 3. 到此说明任务队列有空余，将任务放入任务队列中
    taskQueue_.emplace(sp);
    taskSize_++;
    // 4. 放入新任务，队列不空，通过notEmpty通知
    notEmpty_.notify_all();
}

void ThreadPool::start(int initThreadSize)
{
    // 记录初始线程数量
    initThreadSize_ = initThreadSize;

    // 创建线程
    for(int i = 0; i < initThreadSize_; i++)
    {
        // 创建智能指针对象，通过智能指针在vector析构后销毁new出来的Thread对象
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this));
        // 创建thread线程对象时，将threadPool中线程处理函数threadFunc传递给thread对象
        // 由于unique_ptr删除了copy constructor，只保留了移动构造函数，所以要调用std::move
        threads_.emplace_back(std::move(ptr)); 
    }

    // 启动线程
    for(int i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start();
    }
}

// 线程处理函数，对应消费者模型，在任务队列中处理任务
void ThreadPool::threadFunc()
{
    // std::cout << "begin thread id : " << std::this_thread::get_id() 
    // << std::endl;
    // std::cout << "end thread id : " << std::this_thread::get_id() 
    // << std::endl;

    for(;;)
    {
        std::shared_ptr<Task>task;
        // 添加作用域只能指针lock出了作用域自动释放锁
        {
            // 1. 先获取锁
            std::unique_lock<std::mutex>lock(taskQueMtx_);

            std::cout << "tid:" << std::this_thread::get_id()
				<< "尝试获取任务..." << std::endl;

            // 2. 等待notEmpty条件
            notEmpty_.wait(lock,[&]()->bool{return taskQueue_.size() > 0;});

            std::cout << "执行任务 " << std::endl;

            // 3. 从任务队列中取一个任务出来
            task = taskQueue_.front();
            taskQueue_.pop();
            taskSize_--;

            std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
								<< std::endl;

            // 如果取出任务后，任务队列上还有任务，通知其他线程继续取任务
            if(taskQueue_.size() > 0)
            {
                notEmpty_.notify_all();
            }

            // 取出一个任务后，通知任务提交生产者继续生产任务
            notFull_.notify_all();
        } // 4.线程在执行任务之前应该将锁释放掉，否则其他线程只能干等待无法并发执行

        // 5. 当前线程负责执行这个任务
        if(task != nullptr)
        {
            task->run();
        }
    }
}

//////////////////////////// Thread类定义 //////////////////////////////

// 线程启动函数，创建线程对象并设置线程与线程处理函数分离
void Thread::start()
{
    std::thread t(func_);
    t.detach();
}

// Thread类构造函数
Thread::Thread(ThreadFunc func)
: func_(func)
{}

// Thread类析构函数
Thread::~Thread()
{}