//
// Created by ball on 9/17/22.
//
#include "Timer.h"

std::atomic<int64_t> Timer::s_numCreated_(0);

void Timer::restart(TimeStamp now) {
    //如果是重复的定时器，重新计算下一个超时时刻
    if (repeat_){
        int64_t delta = static_cast<int64_t>(interval_ * TimeStamp::kMicroSecondsPerSecond);
        expiration_ = TimeStamp(now.microSecondsSinceEpoch() + delta);
    }else{
        expiration_ = TimeStamp::invalid();
    }
}
