//
// Created by ball on 9/16/22.
//

#include "TimerQueue.h"
#include "Logger.h"
#include <unistd.h>
#include <sys/timerfd.h>
#include <strings.h>
#include "EventLoop.h"
#include "assert.h"
int createTimerFd() {
    //CLOCK_MONOTONIC参数表明计时器的时间是从系统打开开始计时的
    //CLOCK_MONOTONIC表示的是时间类型
    int timerFd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerFd < 0) {
        LOG_FATAL("Failed in timerfd_create\n");
    }
    return timerFd;
}

// 计算超时时刻与当前时间的时间差
struct timespec howMuchTimeFromNow(TimeStamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch() - TimeStamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / TimeStamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % TimeStamp::kMicroSecondsPerSecond) *1000);
    return ts;
}

// 清除定时器，避免一直触发//处理超时事件。超时后，timerfd变为可读
void readTimerfd(int timerfd,TimeStamp now) {
    uint64_t howmany;
    int n = ::read(timerfd,&howmany,sizeof howmany);
    LOG_INFO("TimerQueue::handleRead()%lu at %s \n",howmany,now.toString().c_str());
    if (n != sizeof howmany) {
        LOG_ERROR("TimerQueue::handleRead() reads %d bytes instead of 8\n",n);
    }
}

// 重置定时器的超时时间（不是周期性的定时器，时间到expiration就结束了）
// 在这里面itimerspec.it_interval都是设置的0，每次都是计时结束以后手动重新设置，然后再计时的。
void resetTimerfd(int timerfd,TimeStamp expiration) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue,sizeof newValue);
    bzero(&oldValue,sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd,0,&newValue,&oldValue);
    if (ret) {
        LOG_ERROR("timerfd_settime()\n");
    }
}

TimerQueue::TimerQueue(EventLoop *loop) :
                loop_(loop),
                timerFd_(createTimerFd()),
                timerFdChannel_(loop,timerFd_),
                timers_(),
                callingExpiredTimer_(false){
    timerFdChannel_.setReadCallback(
            std::bind(&TimerQueue::handleRead,this)
            );
    timerFdChannel_.enableReading();//设置关注读事件，并且加入epoll队列
}

TimerQueue::~TimerQueue() {
    timerFdChannel_.disableAll();
    timerFdChannel_.remove();
    ::close(timerFd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (const Entry& timer:timers_) {
        delete timer.second;
    }

}

TimerId TimerQueue::addTimer(const TimerCallback &cb, TimeStamp when, double interval) {
    Timer *timer =new Timer(std::move(cb),when,interval) ;
//    loop_->runInLoop(
//            std::bind(&TimerQueue::addTimerInLoop,this,std::placeholders::_1)
//            );
    return TimerId(timer,timer->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
//    loop_->runInLoop(
//            std::bind(&TimerQueue::cancelInLoop,this,std::placeholders::_1)
//            );
}

void TimerQueue::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    // 插入一个定时器，有可能会使得最早到期的定时器发生改变
    bool earliestChanged = insert(timer);
    if(earliestChanged) {
        resetTimerfd(timerFd_,timer->expiration());
    }
}

//取消计时器，就是把该计时器从两个队列中删除，
//现在有一种特殊情况，就是如果刚好在处理定时器的过程中，并且这个要取消的定时器就是在被处理的，并且是循环定时器，那么如果不加入cancelingTimers_列表
//就会出现，在重置时又把这个定时器重启了，但是这个定时器应该是要被取消的
void TimerQueue::cancelInLoop(TimerId timerId) {

    assert(timers_.size() == activeTimer_.size());
    ActiveTimer timer(timerId.timer_,timerId.sequence_);
    // // 查找该定时器
    ActiveTimerSet::iterator it  = activeTimer_.find(timer);

    if(it != activeTimer_.end()) {
        size_t n = timers_.erase( Entry(it->first->expiration(), it->first));
        assert(n == 1);
        delete it->first;
        activeTimer_.erase(it);
    }else if(callingExpiredTimer_) {

        //已经到期，并且正在调用回调函数的定时器
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimer_.size());

}

bool TimerQueue::insert(Timer* timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimer_.size());
    bool earliestChanged = false;
    TimeStamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    //如果timer_为空或者when小于timer_最小时间
    if(it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    {
        //插入timers_中
        std::pair<TimerList::iterator,bool> result = timers_.insert(Entry(when, timer));
        assert(result.second);

    }
    {
        //插入到activeTimer_中
        std::pair<ActiveTimerSet::iterator,bool> result = activeTimer_.insert(ActiveTimer(timer,timer->sequence()));
        assert(result.second);
    }
    assert(timers_.size() == activeTimer_.size());
    return earliestChanged;
}

//rov
std::vector<std::pair<TimeStamp,Timer*>> TimerQueue::getExpired(TimeStamp now) {
    assert(timers_.size() == activeTimer_.size());
    std::vector<Entry> expired;
    Entry  sentry(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    //返回第一个未到期的Timer的迭代器，说明之前的定时器全部到期
    // lower_bound含义是返回第一个值大于等于sentry元素的iterater
    //即*end >=sentry，从而end->first >now
    TimerList ::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(),end,back_inserter(expired));
    timers_.erase(timers_.begin(),end);

    for (const Entry& it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimer_.erase(timer);
        assert(n == 1);
    }

    assert(timers_.size() == activeTimer_.size());
    return expired;
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    TimeStamp now(TimeStamp::now());
    readTimerfd(timerFd_,now); //epoll设置的水平触发，清除该事件
    //获取当前时刻之前所有的定时器（即超时定时器列表）
    std::vector<Entry> expired = getExpired(now);
    callingExpiredTimer_ = true;
    cancelingTimers_.clear();

    for(std::vector<Entry>::iterator it = expired.begin();it != expired.end();++it) {
        //执行定时器回调
        it->second->run();
    }
    callingExpiredTimer_ = false;
    //不是一次性定时器 需要重启
    reset(expired,now);
}

void TimerQueue::reset(const std::vector <Entry> &expired, TimeStamp now) {
    TimeStamp nextExpire;

    for (typename std::vector<Entry>::const_iterator it = expired.begin();it != expired.end();++it) {
        ActiveTimer timer(it->second,it->second->sequence());
        //如果是重复的定时器并且 未取消 定时器，则重启该定时器
        if (it->second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it->second->restart(now);
            insert(it->second);
        }
    }

    if(!timers_.empty()) {
        //获取最早到期的定时器超时时间
        nextExpire = timers_.begin()->second->expiration();
    }
    if (nextExpire.valid())
    {
        resetTimerfd(timerFd_, nextExpire);
    }

}