//
// Created by ball on 9/10/22.
//

#include "Acceptor.h"
#include "Logger.h"


Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort):
                        loop_(loop),
                        acceptSocket_(Socket::createNonBlockingFd()),
                        acceptChannel(loop,acceptSocket_.fd()),
                        listening_(false){
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reusePort);
    acceptSocket_.bindAddress(listenAddr);
    //TcpServer::start() Acceptor.listen
    // 如果有新用户的连接，需要执行回调,把connfd打包为channel去唤醒subloop由subLoop来执行具体操作
    acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));

}

Acceptor::~Acceptor() {
    std::cout<<"Acceptor::~Acceptor"<<std::endl;
    acceptChannel.disableAll();
    acceptChannel.remove();
}

void Acceptor::listen() {
    listening_ = true;
    //在fd上监听
    acceptSocket_.listen();
    //poller注册fd可读事件
    acceptChannel.enableReading();
}
//listenFd 有事件发生，即有新连接
void Acceptor::handleRead() {
    InetAddress peerAddr;  //存储客户端地址
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0) {
        if(newConnectionCallback_) {
            newConnectionCallback_(connfd,peerAddr);
        }else {
            ::close(connfd);
        }
    }else {
        LOG_ERROR("Acceptor::handleRead FAILED\n");
        if(errno == EMFILE) {   //文件描述符用完了
            LOG_ERROR("%s:%s:%d handleRead err:%d \n",__FILE__,__FUNCTION__ ,__LINE__,errno);
        }
    }
}