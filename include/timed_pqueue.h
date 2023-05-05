//
// Created by vepbxer on 5/5/23.
//

#ifndef NYSHPORKA_TIMED_PQUEUE_H
#define NYSHPORKA_TIMED_PQUEUE_H
#include <chrono>
#include <functional>
#include <oneapi/tbb/concurrent_priority_queue.h>

inline std::chrono::steady_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<typename T, class Compare = std::greater<std::pair<long, T>>>
class TimedPQueue : public oneapi::tbb::concurrent_priority_queue<std::pair<long, T>, Compare> {
public:
    TimedPQueue() = default;
    TimedPQueue(const TimedPQueue&) = delete;
    TimedPQueue& operator=(const TimedPQueue&) = delete;
    TimedPQueue(TimedPQueue&&) = delete;
    TimedPQueue& operator=(TimedPQueue&&) = delete;

    template<typename U>
    void push(U&& value) {
        auto current_time = get_current_time_fenced().time_since_epoch().count();
        oneapi::tbb::concurrent_priority_queue<std::pair<long, T>, Compare>::push({current_time, std::forward<U>(value)});
    }

    bool try_pop(T& value) {
        std::pair<long, T> temp;
        if (oneapi::tbb::concurrent_priority_queue<std::pair<long, T>, Compare>::try_pop(temp)) {
            value = std::move(temp.second);
            return true;
        }
        return false;
    }

};

#endif //NYSHPORKA_TIMED_PQUEUE_H
