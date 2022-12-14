//
// Created by ball on 9/9/22.
//


#include "stdlib.h"
#include "Poller.h"
#include "EpollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop *loop) {
    if(::getenv("MUDUO_USE_POLL")){
        return nullptr;
    }else {
        return new EpollPoller(loop); //生成epoll实例
    }
}

