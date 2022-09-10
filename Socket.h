//
// Created by ball on 9/10/22.
//

#ifndef MYMUDUO_SOCKET_H
#define MYMUDUO_SOCKET_H
#include "noncopyable.h"
#include "InetAddress.h"

//socket fd
class Socket: noncopyable{
public:
    explicit Socket(int sockFd):sockFd_(sockFd){}
    ~Socket();

    int fd() const {return sockFd_;}
    void bindAddress(const InetAddress &localAddress);
    void listen();
    int accept(InetAddress *peerAddress);
    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockFd_;

};

#endif //MYMUDUO_SOCKET_H
