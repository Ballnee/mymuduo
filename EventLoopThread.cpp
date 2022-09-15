//
// Created by ball on 9/10/22.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name):
                                loop_(nullptr),
                                exiting_(false),
                                thread_(std::bind(&EventLoopThread::threadFunc, this),name),
                                mutex_(),
                                cond_(),
                                callback_(cb){}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_!= nullptr) {
        loop_->quit();//退出循环

        thread_.join();//等待子线程结束
    }
}

EventLoop *EventLoopThread::startLoop() {
    thread_.start();//执行底层新线程, 每一次start都是启动一个新线程

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}
//这个方法是在新线程里面运行的
void EventLoopThread::threadFunc() {
    EventLoop loop; //创建一个独立的EventLoop对象，和上面启动的新线程是对应的 one loop pre thread

    if(callback_) {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    //开始监听读写事件
    loop.loop();//EventLoop loop -> Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;  //退出事件循环
}