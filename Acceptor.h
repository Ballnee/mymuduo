//
// Created by ball on 9/10/22.
//

#ifndef MYMUDUO_ACCEPTOR_H
#define MYMUDUO_ACCEPTOR_H

#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"
#include "functional"

class Acceptor : noncopyable {
public:
    using NewConnectionCallback= std::function<void(int sockfd,const InetAddress&)>;
    Acceptor(EventLoop *loop,const InetAddress &listenAddr,bool reusePort);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb){newConnectionCallback_ = std::move(cb);}

    bool listening()const {return listening_;}
    void listen();
private:
    void handleRead();
    EventLoop *loop_;
    Socket  acceptSocket_;
    Channel  acceptChannel; // clientfd conn success
    NewConnectionCallback newConnectionCallback_;
    bool listening_;

};


#endif //MYMUDUO_ACCEPTOR_H
