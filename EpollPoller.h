//
// Created by ball on 9/9/22.
//

#ifndef MYMUDUO_EPOLLPOLLER_H
#define MYMUDUO_EPOLLPOLLER_H

#include "Poller.h"
#include "vector"
#include "sys/epoll.h"

class EpollPoller : Poller{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;
    //重写基类的抽象方法
    TimeStamp poll(int timeoutMs,ChannelList *activeChannels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;
//    填写活跃的连接
    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
//    更新channel通道
    void update(int operation,Channel* channel);

    using EventList = std::vector<epoll_event>;
    int epollFd_;
    EventList events_;
};




#endif //MYMUDUO_EPOLLPOLLER_H
