//
// Created by ball on 9/8/22.
//

#include "TcpServer.h"
#include "Logger.h"

static EventLoop* CheckLoopNotNuLL(EventLoop *loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n",__FILE__,__FUNCTION__ ,__LINE__);
    }
    return loop;
}
TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option):
                    loop_(CheckLoopNotNuLL(loop)),
                    ipPort_(listenAddr.toIpPort()),
                    name_(nameArg),
                    acceptor_(new Acceptor(loop,listenAddr,option == kReusePort)),
                    threadPool_(new EventLoopThreadPool(loop,nameArg)),
                    connectionCallback_(),
                    messageCallback_(),
                    nextConnId_(1){
    //写入新用户连接时的回调函数
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                  this,std::placeholders::_1,std::placeholders::_2));

}

void TcpServer::start() {
    if(started_++ == 0) {  //atomic
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}



void TcpServer::newConnection(int sockFd, const InetAddress &peerAddr) {

}