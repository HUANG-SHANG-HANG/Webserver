#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

class ThreadPool {
public:
    ThreadPool(int threadCount = 4) : stop_(false) {
        for (int i = 0; i < threadCount; i++) {
            workers_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        cond_.wait(lock, [this]() {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cond_.notify_all();
        for (auto& t : workers_) {
            t.join();
        }
    }

    void addTask(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            tasks_.push(std::move(task));
        }
        cond_.notify_one();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cond_;
    bool stop_;
};

#endif

