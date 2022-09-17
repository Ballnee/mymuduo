//
// Created by ball on 9/16/22.
//

#ifndef MYMUDUO_TIMERQUEUE_H
#define MYMUDUO_TIMERQUEUE_H
#include "set"
#include "vector"
#include "noncopyable.h"
#include "mutex"
#include "TimeStamp.h"
#include "Callbacks.h"
#include "Channel.h"
#include "Timer.h"
#include "memory"
#include "TimerId.h"
class TimerQueue : noncopyable {
public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback &cb,TimeStamp when,double interval);

    void cancel(TimerId timerId);
private:
    // unique_ptr是C++11 标准的一个独享所有权的智能指针
    // 无法得到指向同一对象的两个unique_ptr指针
    // 但可以进行移动构造与移动赋值操作，即所有权可以移动到另一个对象（而非拷贝构造）
    //set中存储的是pair类型，那么默认先按照pair的第一个元素排序，如果相同，再按照第二个元素排序。
    //所以这两种set都是存放定时器的列表，但是一个根据定时器的到时时间来存储，
    //一个根据定时器地址来存储，但是存储的定时器都是同一个，目的是为了区分同一到期时间的定时器？？？
    //支持定时器时间相同
    //Entry 时间戳和定时器对象的地址（定时器对象包括，超时时刻，回调函数，定时器序号，是否需要重复定时标志等）
    using Entry = std::pair<TimeStamp,Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*,int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;


    // 以下成员函数只可能在其所属的I/O线程中调用，因而不必加锁。
    // 服务器性能杀手之一是锁竞争，所以要尽可能少用锁
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead(); //timerfdChannel_的读函数
    //返回超时的定时器列表
    std::vector<Entry> getExpired(TimeStamp now);
//    对超时的定时器重置，因为可能存在重复定时器
    void reset (const std::vector<Entry> &expired,TimeStamp now);

    bool insert(Timer* timer);

    EventLoop *loop_; // 所属EventLoop

    const int timerFd_;
    //过一段事件，就筛选一次，看看TimerList中有多少定时器到时间了，就处理一下，但是这样延迟很高，不太理解
    Channel timerFdChannel_;

    TimerList  timers_; // timers_是按到期时间排序，也是存放未到时间的定时器
    // for cancel()
    // timers_与activeTimers_保存的是相同的数据
    // timers_是按到期时间排序，activeTimers_是按对象地址排序
    ActiveTimerSet activeTimer_;

    bool callingExpiredTimer_; /* atomic *///是否在处理过期定时器的标志

    ActiveTimerSet cancelingTimers_;    // 保存的是被取消的定时器
                                        // 用这个列表的作用是，当出现一个循环的计时器被取消时，就要通过reset函数中对
                                        //ActiveTimerSet列表来暂停对这个计时器的重置

};
#endif //MYMUDUO_TIMERQUEUE_H
