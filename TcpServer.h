//
// Created by ball on 9/8/22.
//

/*
 * 
 * 
 * 
 * */
#ifndef MYMUDUO_TCPSERVER_H
#define MYMUDUO_TCPSERVER_H
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "Callbacks.h"
#include "atomic"
#include "unordered_map"
#include "cstring"

class TcpServer : noncopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option {
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg,Option option = kNoReusePort);
    ~TcpServer();
    //
    //设置subLoop的个数，即线程数
    void setThreadNum(int numThreads);

    void setThreadInitCallback(const ThreadInitCallback& cb){threadInitCallback_ = std::move(cb);}
    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_ = std::move(cb);}
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = std::move(cb);}

    void start();

private:
    void newConnection(int sockFd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string,TcpConnectionPtr>;
    EventLoop *loop_;

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_; //mainLoop主要是监听新的连接事件
    std::shared_ptr<EventLoopThreadPool> threadPool_; //one thread per loop

    ConnectionCallback connectionCallback_; //有新连接时的回调
    MessageCallback messageCallback_; //有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_;//消息发送完成的回调

    ThreadInitCallback threadInitCallback_; //loop线程初始化的回调

    std::atomic<int> started_;

    ConnectionMap connectionMap_;//保存所有的连接

    int nextConnId_;
    
};




#endif //MYMUDUO_TCPSERVER_H
