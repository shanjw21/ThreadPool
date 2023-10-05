#include "threadPool.h"
#include <thread>
#include <chrono>

class Mytask : public Task{
public:

    Mytask(int begin, int end)
    : begin_(begin)
    , end_(end)
    {}

    // 1. 如何设计run函数的返回值，使其可以表示任意类型
    void run()
    {
        std::cout << "tid : " << std::this_thread::get_id()
        << "begin: " << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "tid : " << std::this_thread::get_id()
        << "end: " << std::endl;
    }
private:
    int begin_;
    int end_;
};

int main()
{
    ThreadPool pool;
    pool.start();
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    getchar();
    return 0;
}