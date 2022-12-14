//
// Created by ball on 9/8/22.
//

#include "Channel.h"
#include "Logger.h"
#include "sys/epoll.h"
#include "EventLoop.h"
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI ;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd): loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1),tied_(false) {}

Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void> &obj) {

    tie_ = obj;
    tied_ = true;
}
//当改变channel所代表fd的events事件后，update负责在poller里面更改相应的fd的epool_ctl
//Eventloop => channelList poller
void Channel::update() {
    //通过channel所属的EventLoop调用pooler相应的方法 注册fd的event事件
    loop_->updateChannel(this);
}

//在channel所属的EventLoop中，把当前的channel删除掉
void Channel::remove() {
    loop_->removeChannel(this);
}
void Channel::handleEvent(TimeStamp receiveTime) {
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if(guard) {
            handleEventWithGuard(receiveTime);
        }
    }else {
        handleEventWithGuard(receiveTime);
    }
}
//根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作；
void Channel::handleEventWithGuard(TimeStamp receiveTime) {
    LOG_INFO("Channel handleEvent revents:%d",revents_);
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if(closeCallback_) {
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR) {
        if(errorCallback_) {
            errorCallback_();
        }
    }
    if(revents_ &(EPOLLIN | EPOLLPRI)){
        if(readEventCallback_) {
            readEventCallback_(receiveTime);
        }
    }
    if(revents_ & EPOLLOUT) {
        if(writeCallback_) {
            writeCallback_();
        }
    }
}
