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

template<typename T, class Compare = std::greater<std::pair<std::pair<long, long>, T>>>
class TimedPQueue : public oneapi::tbb::concurrent_priority_queue<std::pair<std::pair<long, long>, T>, Compare> {
    using elem = std::pair<std::pair<long, long>, T>;
    using base = oneapi::tbb::concurrent_priority_queue<elem, Compare>;
public:
    TimedPQueue() = default;
    TimedPQueue(const TimedPQueue&) = delete;
    TimedPQueue& operator=(const TimedPQueue&) = delete;
    TimedPQueue(TimedPQueue&&) = delete;
    TimedPQueue& operator=(TimedPQueue&&) = delete;

    template<typename U>
    void push(U&& value, long priority = 0) {
        auto current_time = get_current_time_fenced().time_since_epoch().count();
        base::push({std::make_pair(priority, current_time), std::forward<U>(value)});
    }

    bool try_pop(T& value, long& priority) {
        elem temp;
        if (base::try_pop(temp)) {
            value = std::move(temp.second);
            priority = temp.first.first;
            return true;
        }
        return false;
    }

};

#endif //NYSHPORKA_TIMED_PQUEUE_H
