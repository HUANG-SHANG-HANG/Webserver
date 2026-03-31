#ifndef TIMER_H
#define TIMER_H

#include <unordered_map>
#include <vector>
#include <chrono>
#include <functional>
#include <mutex>

using TimeoutCallback = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using TimeStamp = Clock::time_point;

struct TimerNode {
    int fd;
    TimeStamp expire;
    TimeoutCallback cb;
};

class Timer {
public:
    void add(int fd, int timeoutMs, const TimeoutCallback& cb);
    void remove(int fd);
    void tick();
    int getNextTick();

    std::mutex mtx;  // 公开锁，主线程和工作线程都要用

private:
    void swapNode(int i, int j);
    void siftUp(int i);
    void siftDown(int i, int n);
    void delNode(int index);

    std::vector<TimerNode> heap_;
    std::unordered_map<int, int> fdToIndex_;
};

#endif

