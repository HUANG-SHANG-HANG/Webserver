#include "timer/Timer.h"

void Timer::swapNode(int i, int j) {
    std::swap(heap_[i], heap_[j]);
    fdToIndex_[heap_[i].fd] = i;
    fdToIndex_[heap_[j].fd] = j;
}

void Timer::siftUp(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap_[parent].expire <= heap_[i].expire) break;
        swapNode(i, parent);
        i = parent;
    }
}

void Timer::siftDown(int i, int n) {
    while (true) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < n && heap_[left].expire < heap_[smallest].expire)
            smallest = left;
        if (right < n && heap_[right].expire < heap_[smallest].expire)
            smallest = right;
        if (smallest == i) break;
        swapNode(i, smallest);
        i = smallest;
    }
}

void Timer::delNode(int index) {
    int n = heap_.size() - 1;
    if (index < n) {
        swapNode(index, n);
        fdToIndex_.erase(heap_.back().fd);
        heap_.pop_back();
        if (!heap_.empty() && index < (int)heap_.size()) {
            siftDown(index, heap_.size());
            siftUp(index);
        }
    } else {
        fdToIndex_.erase(heap_.back().fd);
        heap_.pop_back();
    }
}

void Timer::add(int fd, int timeoutMs, const TimeoutCallback& cb) {
    auto expire = Clock::now() + std::chrono::milliseconds(timeoutMs);

    if (fdToIndex_.count(fd)) {
        // 已存在，更新超时时间
        int idx = fdToIndex_[fd];
        heap_[idx].expire = expire;
        heap_[idx].cb = cb;
        siftDown(idx, heap_.size());
        siftUp(idx);
    } else {
        // 新增
        fdToIndex_[fd] = heap_.size();
        heap_.push_back({fd, expire, cb});
        siftUp(heap_.size() - 1);
    }
}

void Timer::remove(int fd) {
    if (fdToIndex_.count(fd)) {
        delNode(fdToIndex_[fd]);
    }
}

void Timer::tick() {
    while (!heap_.empty()) {
        if (heap_.front().expire > Clock::now()) break;
        heap_.front().cb();
        delNode(0);
    }
}

int Timer::getNextTick() {
    if (heap_.empty()) return -1;
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
        heap_.front().expire - Clock::now());
    return diff.count() > 0 ? diff.count() : 0;
}

