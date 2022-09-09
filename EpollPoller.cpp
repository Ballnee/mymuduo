//
// Created by ball on 9/9/22.
//

#include "EpollPoller.h"
#include "Logger.h"
#include "errno.h"
#include <unistd.h>
#include "Channel.h"
#include "EventLoop.h"
#include "string.h"

//kNew 表示channel未添加到poller中
const int kNew = -1;  //channel index_ = -1;
//表示channel已添加到poller中
const int kAdded = 1;
//表示channel从poller中删除
const int kDeleted = 2;


EpollPoller::EpollPoller(EventLoop *loop):
                Poller(loop)
                ,epollFd_(::epoll_create1(EPOLL_CLOEXEC))
                ,events_(kInitEventListSize) {
    if (epollFd_ < 0) {
        LOG_FATAL("epoll_create error:%d\n",errno);
    }
}

EpollPoller::~EpollPoller()  {
    ::close(epollFd_);
}

TimeStamp EpollPoller::poll(int timeoutMs,ChannelList *activeChannels){
    LOG_INFO("[EpollPoller::poll: fd total count:%d\n",static_cast<int>(channels_.size()));
    int numEvents = ::epoll_wait(epollFd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno = errno;
    TimeStamp now(TimeStamp::now());
    if(numEvents > 0) {
        LOG_INFO("%d events happened \n",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents == events_.size()) {
            events_.resize(events_.size()*2);
        }
    }else if(numEvents == 0) {
        LOG_DEBUG("%s timeout!\n",__FUNCTION__);
    }else {
        if(saveErrno != EINTR) {
            errno = saveErrno; //keep error num
            LOG_ERROR("%s",__FUNCTION__ );
        }
    }
    return now;
}

//channel update remove  ->EventLoop updateChannel removeChannel ->Poller updateChannel removeChannel
/*
 *              EventLoop   -> poller.poll
 *    ChannelList           Poller
 *                       ChannelMap  <fd,channel*>
 *
 *
 *
 * */
void EpollPoller::updateChannel(Channel *channel){
    const int index = channel->index();
    LOG_INFO("[EpollPoller.updateChannel:fd=%d events=%d index=%d \n",channel->fd(),channel->events(),index);
    //channel没有在poller上注册过或者注册过但是已删除了
    if (index == kNew || index == kDeleted) {
        if(index == kNew){
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }else { //channel在poller上面注册过
        int fd = channel->fd();
        if(channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }else {
            update(EPOLL_CTL_MOD,channel);
        }

    }
}


//从pooler中删除channel
void EpollPoller::removeChannel(Channel *channel){
    int fd = channel->fd();
    channels_.erase(fd);
    int index = channel->index();
    if(index == kAdded) {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels) const{
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);//EventLoop拿到了发生事件的channel列表
    }
}

void EpollPoller::update(int operation,Channel* channel){
    epoll_event event;
    memset(&event,0,sizeof event);
    int fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = channel;
    event.data.fd = fd;

    if (::epoll_ctl(epollFd_,operation,fd,&event) < 0) {
        if (operation == EPOLL_CTL_DEL){
            LOG_ERROR("[EpollPoller::update:epoll_ctl_del error:%d",errno);
        }else {
            LOG_FATAL("[EpollPoller::update:epoll_ctl add/mod error:%d\n",errno);
        }
    }

}
