//
// Created by ball on 9/10/22.
//

#include "EventLoop.h"
#include "EventLoopThread.h"
#include "noncopyable.h"
#ifndef MYMUDUO_EVENTLOOPTHREADPOOL_H
#define MYMUDUO_EVENTLOOPTHREADPOOL_H
class EventLoopThreadPool : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop *baseLoop,const std::string &nameArg);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads){numThreads_ = numThreads;}
    void start(const ThreadInitCallback &cb=ThreadInitCallback());
//如果工作在多线程的模式下，baseLoop默认会以轮询的方式分配channel给subLoop
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();

    bool started()const{return started_;}
    const std::string& name() const {return name_;}

private:
    EventLoop *baseLoop_;   //用户创建的一个loop，主线程
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

};
#endif //MYMUDUO_EVENTLOOPTHREADPOOL_H
