//
// Created by ball on 9/10/22.
//

#ifndef MYMUDUO_EVENTLOOPTHREAD_H
#define MYMUDUO_EVENTLOOPTHREAD_H

#include "noncopyable.h"
#include "Thread.h"
#include "mutex"
#include "condition_variable"
#include "functional"
#include "string"

class EventLoop;
class EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),const std::string &name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();


private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;

    std::mutex  mutex_;
    std::condition_variable cond_;

    ThreadInitCallback callback_;

};


#endif //MYMUDUO_EVENTLOOPTHREAD_H
