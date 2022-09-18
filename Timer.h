//
// Created by ball on 9/16/22.
//

#ifndef MYMUDUO_TIMER_H
#define MYMUDUO_TIMER_H
#include "noncopyable.h"
#include "Callbacks.h"
#include "TimeStamp.h"
#include "atomic"
#include "mutex"
#include "memory"

class Timer : noncopyable {
public:
    Timer(const TimerCallback &cb,TimeStamp when,double interval)
    :callback_(std::move(cb)),
    expiration_(when),
    interval_(interval),
    repeat_(interval>0.0),
    sequence_(++s_numCreated_)
    {

    }

    void run(){
        callback_();
    }

    TimeStamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(TimeStamp now);

    static int64_t numCreated() {
        return s_numCreated_;
    }

private:
    const TimerCallback callback_;  //定时器回调函数
    TimeStamp expiration_;          // 下一次的超时时间
    const double interval_;         //超时时间间隔，如果是一次性定时器，该值为0
    const bool repeat_;             //是否需要重fu发生
    const int64_t sequence_;        // 定时器序号，不会重


    std::mutex latch_;
    static std::atomic<int64_t>     s_numCreated_; //定时器计数，当前已经创建的定时器数量
};
#endif //MYMUDUO_TIMER_H
