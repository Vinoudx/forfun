
#include <condition_variable>
#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <memory>
#include <algorithm>

class ThreadPool{

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable condition;
    bool stop;

public:

    ThreadPool(int num):stop(false){
        for (size_t i = 0; i < num; i++){
            threads.emplace_back([this,i]{
                while (true){
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock,[this,i]{
                        return !tasks.empty() || stop;
                    });
                    if(stop && tasks.empty())return;
                    std::function<void()> task;
                    task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }

    ~ThreadPool(){
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        condition.notify_all();
        for(auto &t : threads){
            t.join();
        }
    }

    //这个接口需要重新设计
    template<typename Res_ ,typename Fun_, typename ...Args>
    std::future<Res_> submit(Fun_&& func , Args&& ... args){//直接在这里加priority
        auto task = std::make_shared<std::packaged_task<Res_(Args...)>>
            (std::bind(std::forward<Fun_>(func) , std::forward<Args>(args)... ));
        std::future<Res_> res = task->get_future();
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace([task,args...]{
            (*task)(args...);//这里居然还要传参数
        });
        lock.~unique_lock();
        condition.notify_one();
        return res;
    }

    void close(){
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
    }
    
};
