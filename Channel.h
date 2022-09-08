//
// Created by ball on 9/8/22.
//

#ifndef MYMUDUO_CHANNEL_H
#define MYMUDUO_CHANNEL_H
//Channel 理解为通道 封装了sockfd和感兴趣的event 如 epollin，epollout等事件，还绑定了poller返回的具体事件
#include "noncopyable.h"
#include "TimeStamp.h"
#include "functional"
#include <iostream>
#include "memory"
class EventLoop;
class TimeStamp;
class Channel : noncopyable {
public:
    using EventCallback = std::function<void()> ;
    using ReadEventCallback = std::function<void(TimeStamp)>;

    Channel(EventLoop *loop,int fd);
    ~Channel();
    //fd得到poller通知以后处理事件的
    void handleEvent(TimeStamp receiveTime);

    void setReadCallback(ReadEventCallback cb) {
        readEventCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb) {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb) {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb) {
        errorCallback_ = std::move(cb);
    }

    //防止当Channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_ ;}
    int events() const {return events_;}
    int set_revents(int revt) {revents_ = revt;}

    //设置对哪些事件感兴趣  update add fd to rbtree
    void enableReading(){events_ |= kReadEvent;update();}
    void disableReading(){events_ &=~kReadEvent;update();}
    void enableWriting(){events_ |=kWriteEvent;update();}
    void disableWriting(){events_ &= ~kWriteEvent;update();}
    void disableAll() {events_ = kNoneEvent;update();}

    bool isNoneEvent() const {return events_ == kNoneEvent;}
    bool isWriting() const {return  events_&kWriteEvent;}
    bool isReading() const {return events_&kReadEvent;}

    int index() {return index_;}

    void set_index(int index){index_=index;}

    //one loop per thread
    EventLoop* ownerLoop() {return loop_;}

    void remove();

private:
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;//事件循环
    const int fd_;//poller监听的对象
    int events_;//注册fd感兴趣的事件
    int revents_;
    int index_;
    std::weak_ptr<void> tie_;
    bool tied_;

    //因为Channel 通道里面可以获知fd最终具体发生的具体事件revents，所以它负责具体事件的回调操作
    ReadEventCallback readEventCallback_;
    EventCallback   writeCallback_;
    EventCallback   closeCallback_;
    EventCallback   errorCallback_;
};
#endif //MYMUDUO_CHANNEL_H
