//
// Created by ball on 9/10/22.
//

#ifndef MYMUDUO_TCPCONNECTION_H
#define MYMUDUO_TCPCONNECTION_H
#include "Channel.h"
#include "EventLoop.h"
#include "noncopyable.h"
#include "Socket.h"
#include "string"
#include "atomic"
#include "Buffer.h"
#include "memory"
#include "Callbacks.h"

/*
 * TcpServer->Acceptor 有一个新连接拿到connfd
    ->TcpConnection 设置回调 ->channel ->poller->channel的回调操作
 *
 *
 * */

class TcpConnection : noncopyable,public std::enable_shared_from_this<TcpConnection>{
public:

    TcpConnection(EventLoop *loop,const std::string& name,int sockFd,const InetAddress& localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop()const {return loop_;}
    const std::string getName() const {return name_;}
    const InetAddress& localAddress() const {return localAddr_;}
    const InetAddress& peerAddress() const {return peerAddr_;}

    bool connected()const {return  state_ == kConnected;}

    void send(const std::string& buf);
    void shutdown();
    void shutdownInLoop();

    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = std::move(cb);}
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = std::move(cb);}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {writeCompleteCallback_ = std::move(cb);}
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,size_t highWaterMark) {
        highWaterMarkCallback_ = std::move(cb);
        highWaterMark_ = highWaterMark;
    }
    void setCloseCallback(const CloseCallback& cb) {closeCallback_ = std::move(cb);}

    void connectEstablished();
    void connectDestroyed();





private:
    enum StateE {kDisconnected,kConnecting,kConnected,kDisconnecting};
    void setState(StateE stateE) {state_ = stateE;}
    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message,size_t len);

    EventLoop *loop_;//这里绝对不是baseLoop，因为TcpConnection都在subLoop中管理
    const std::string name_;
    std::atomic<StateE> state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress  localAddr_;
    const InetAddress  peerAddr_;

    ConnectionCallback connectionCallback_; //有新连接时的回调
    MessageCallback messageCallback_; //有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_;//消息发送完成的回调
    HighWaterMarkCallback highWaterMarkCallback_; //
    CloseCallback closeCallback_;

    size_t highWaterMark_;

    Buffer inputBuffer_;  //接收数据的缓冲区

    Buffer outputBuffer_;//发送数据的缓冲区  send成员函数往outputBuffer里面发数据


};
#endif //MYMUDUO_TCPCONNECTION_H
