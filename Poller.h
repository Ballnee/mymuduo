//
// Created by ball on 9/9/22.
//

#ifndef MYMUDUO_POLLER_H
#define MYMUDUO_POLLER_H

#include "noncopyable.h"
#include "vector"
#include "cstdio"
#include "iostream"
#include "unordered_map"
#include "TimeStamp.h"
#include "EventLoop.h"
class Channel;
class EventLoop;

class Poller :noncopyable {
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop *loop);
    virtual ~Poller() = default;
    virtual TimeStamp poll(int timeoutMs,ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) =0;
    bool hasChannel(Channel *channel) const;
    //EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
//    map的key表示的是socketfd，value就是sockfd所属的通道
//     protected外界无法访问但是派生类可以访问
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};


#endif //MYMUDUO_POLLER_H
