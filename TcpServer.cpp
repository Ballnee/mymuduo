//
// Created by ball on 9/8/22.
//

#include "TcpServer.h"
#include "TcpConnection.h"
#include "Logger.h"
#include "strings.h"
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
                    nextConnId_(1),
                    started_(0){
    //写入新用户连接时的回调函数
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                  this,std::placeholders::_1,std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for(auto &item : connectionMap_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed,conn)
                );
    }
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


//有一个新的客户端连接，就执行这个回调
void TcpServer::newConnection(int sockFd, const InetAddress &peerAddr) {
    //轮询选择一个subloop  来管理channel
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf,sizeof buf,"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("tcpServer::newConnection [%s] - new connection [%s] from %s\n",
             name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());
    //通过sockfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local,sizeof local);
    socklen_t addrlen = sizeof local;
    if (::getsockname(sockFd,(sockaddr*)&local,&addrlen) < 0) {
        LOG_ERROR("socket::getLocalAddr");
    }
    InetAddress localAddr;

    //根据连接成功的sockfd，创建TcpConnection连接对象

    TcpConnectionPtr tcpConnectionPtr(new TcpConnection(ioLoop,connName,sockFd/*Socket,channel*/,localAddr,peerAddr));
    connectionMap_[connName] = tcpConnectionPtr;
    //下面的回调函数都是用户设置的,->
    //tcpServer ->  tcpConnection -> channel -> poller notify channel callback
    tcpConnectionPtr->setConnectionCallback(connectionCallback_);
    tcpConnectionPtr->setMessageCallback(messageCallback_);
    tcpConnectionPtr->setWriteCompleteCallback(writeCompleteCallback_);
    //设置关闭连接的回调
    tcpConnectionPtr->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
   //直接调用
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,tcpConnectionPtr));

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(
            std::bind(&TcpServer::removeConnectionInLoop,this,conn)
            );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n",name_.c_str(),conn->getName().c_str());

    connectionMap_.erase(conn->getName());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn)
            );
}