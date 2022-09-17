//
// Created by ball on 9/16/22.
//

#ifndef MYMUDUO_TIMERID_H
#define MYMUDUO_TIMERID_H

#include "Timer.h"
#include "memory"

class TimerId {
public:
    TimerId() : timer_(nullptr),sequence_(0) {}
    TimerId(Timer* timer,int64_t seq) : timer_(timer),sequence_(seq) {}

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};
#endif //MYMUDUO_TIMERID_H
