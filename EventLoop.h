//
// Created by ball on 9/8/22.
//

#ifndef MYMUDUO_EVENTLOOP_H
#define MYMUDUO_EVENTLOOP_H
class Channel;
class Poller;
class TimerQueue;
#include "Logger.h"
#include "functional"
#include "noncopyable.h"
#include "vector"
#include "atomic"
#include "memory"
#include "TimeStamp.h"
#include "TimerQueue.h"
#include "mutex"
#include "CurrentThread.h"
//事件循环类  主要包含了两个大模块  Channel  Poller （epoll的抽象）
class EventLoop : noncopyable{
public:
    using Functor = std::function<void()>;

    EventLoop();

    ~EventLoop();
    //开启事件循环
    void loop();
    //退出事件循环
    void quit();
    TimerId runAt(const TimeStamp& time,const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval,const TimerCallback& cb);
    void cancel(TimerId timerId);
    //在当前loop中执行cb
    void runInLoop(Functor cb);
    //把cb放入队列，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);
    //唤醒loop所在的线程
    void wakeup();
    //EventLoop的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
    //EventLoop线程的线程id和当前线程（即调用当前成员函数的线程）的线程id
    //相当于A线程创建了一个对象，A自己去执行对象的成员函数或者A线程开启B线程去执行对象的成员函数
    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
    void assertInLoopThread(){
        if (!isInLoopThread()) {
            LOG_FATAL(" Current thread id :%d is not as same as this obj thread id :%d\n",CurrentThread::tid(),threadId_);
        }
    }

private:
    void handleRead();  //wake up
    void doPendingFunctors(); // 执行回调
    using ChannelList = std::vector<Channel*>;
    std::atomic_bool  looping_;//原子操作，通过CAS实现
    std::atomic_bool quit_; //标识退出loop循环

    const pid_t threadId_; // 记录当前loop的线程id
    TimeStamp pollReturnTime_;// poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;//主要作用，当mainLoop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::atomic_bool  callingPendingFunctors_;//标识当前loop是否需要执行回调
    std::vector<Functor> pendingFunctors_; //存储loop需要执行的回调操作
    std::mutex mutex_; //互斥锁，用来保证上面vector容器的线程安全


};
#endif //MYMUDUO_EVENTLOOP_H
