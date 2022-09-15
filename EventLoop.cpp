//
// Created by ball on 9/8/22.
//

#include "EventLoop.h"
#include "sys/eventfd.h"
#include "unistd.h"
#include "fcntl.h"
#include "Logger.h"
#include "Poller.h"
#include "errno.h"
#include "Channel.h"
#include "memory"
//防止一个线程创建多个EventLoop  likely thread_local
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller io复用接口的超时时间
const int kPollTimeMs = 10000;

//
//创建wakeupfd，用notify唤醒subReactor处理新来的channel
int createEventFd(){
    //not use socket fd
    int evtfd = ::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evtfd < 0) {
        LOG_FATAL("eventfd error:%d\n",errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
        :looping_(false),
        quit_(false),
        callingPendingFunctors_(false),
        threadId_(CurrentThread::tid()),
        poller_(Poller::newDefaultPoller(this)),
        wakeupFd_(createEventFd()),
        wakeupChannel_(new Channel(this,wakeupFd_)),
        currentActiveChannel_(nullptr){
            LOG_DEBUG("EventLoop created %p in thread%d\n",this,threadId_);
            if(t_loopInThisThread) {
                LOG_FATAL("Another EventLoop %p exists in this thread %d \n",t_loopInThisThread,threadId_);
            }else {
                t_loopInThisThread = this;
            }
        //设置wakeupFd感兴趣的事件和发生事件的回调操作
        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
        //每个Eventloop都讲监听wakeupchannel的EPOLLLIN读事件了
        wakeupChannel_->enableReading();

}

EventLoop::~EventLoop() {
    std::cout<<"EventLoop::~EventLoop()"<<std::endl;
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop:: loop() {
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping \n", this);
    while(!quit_) {
        activeChannels_.clear();
        //监听两类fd，一种是client的fd，一种是wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
        for (Channel *channel:activeChannels_) {
           // Poller监听哪些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop事件循环需要处理的回调操作
        //
        /*  IO线程 mainLoop 主要处理accept，接受新用户的连接，然后accept返回一个和客户通信的fd，使用一个channel来打包fd
            所以对于已经连接的fd，channel需要分发给 subLoop
            即mainLoop 需要对已连接的操作时，需要唤醒subLoop去操作
            mainLoop 事先注册一个回调cb， 需要subLoop来执行   wakeup之后subLoop才起来了
         * */
        //mainLoop 需要subLoop执行的回调函数，可能需要执行多个
        doPendingFunctors();
    }

        LOG_INFO("EventLoop %p stop looping\n", this);
        looping_ = false;

}
//在当前线程中执行cb
void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {//在当前线程中执行cb
        LOG_INFO("this thread is myself\n");
        cb();
    }else {  //在非当前线程中执行cb，则需要唤醒loop所在线程 执行cb
        LOG_INFO("this thread is not  myself\n");
        queueInLoop(cb);
        LOG_INFO("add connection callback\n");
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    //唤醒相应的，需要执行上面的回调操作的loop线程
    //|| callingPendingFunctors_的意思是，当前loop正在执行回调，但是现在的loop又有了新的回调，为了防止阻塞，需要唤醒
    if(!isInLoopThread() || callingPendingFunctors_) { //
        wakeup();//唤醒loop所在线程
    }
}

//用来唤醒loop所在线程，向wakeupfd_写一个数据，wakeupChannel就发生读事件，当前loop线程就被唤醒
void EventLoop::wakeup() {
    uint64_t  one = 1;
    ssize_t n = write(wakeupFd_,&one,sizeof one);
    if(n != sizeof one) {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n",n);
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);

    }
    for(const Functor &functor:functors) {
        functor();//执行当前loop需要执行的回调操作
    }
    callingPendingFunctors_ = false;
}


//退出事件循环 1.loop在自己的线程中调用quit  2.在其他线程中调用quit
void EventLoop::quit() {
    quit_ = true;
    if(!isInLoopThread()) {
        wakeup();  //如果是其他线程中，调用的quit ， 在一个subloop（worker）中，调用了mainLoop（io）的quit，需要把主线程唤醒
    }

}

void EventLoop::handleRead() {
    uint64_t one = 1;
    int n = read(wakeupFd_,&one,sizeof one); //一个读操作用来唤醒subLoop

    if(n != sizeof one) {
        LOG_ERROR("EventLoop:handleRead() reads %d bytes instead of 8\n",n);
    }
}

