//
// Created by ball on 9/10/22.
//

#include "unistd.h"
#include "Socket.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "Logger.h"
#include "strings.h"
#include "netinet/tcp.h"
Socket::~Socket() {
    close(sockFd_);
}

void Socket::bindAddress(const InetAddress &localAddress) {

    if(0 != bind(sockFd_,(sockaddr*)localAddress.getSockAddr(),sizeof(sockaddr_in))){
        LOG_FATAL("bind sockfd:%d failed\n",sockFd_);
    }
}

void Socket::listen() {
    if (0 != ::listen(sockFd_,1024)) {
        LOG_FATAL("listen socket:%d failed \n",sockFd_);
    }
}

int Socket::accept(InetAddress *peerAddress) {
    sockaddr_in addr;
    socklen_t len;
    bzero(&addr,sizeof addr);
    int connFd = ::accept(sockFd_,(sockaddr*)&addr,&len);
    if (connFd >= 0) {
        peerAddress->setSockAddr(addr);
    }
    return connFd;
}

int Socket::createNonBlockingFd() {
    int sockFd = ::socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,IPPROTO_TCP);
    if(sockFd < 0) {
        LOG_FATAL("cSocket::createNonBlockingFd failed\n");
    }
    return sockFd;
}

void Socket::shutdownWrite() {
    if (::shutdown(sockFd_,SHUT_WR) < 0) {
        LOG_ERROR("sockets::shutdownWrite!");
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on?1:0;
    ::setsockopt(sockFd_,IPPROTO_TCP,TCP_NODELAY,&optval,static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) {
    int optval = on?1:0;
    ::setsockopt(sockFd_,SOL_SOCKET,SO_REUSEADDR,&optval,static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
    int optval = on?1:0;
    ::setsockopt(sockFd_,SOL_SOCKET,SO_REUSEPORT,&optval,static_cast<socklen_t>(sizeof optval));
}

void Socket::setKeepAlive(bool on) {
    int optval = on?1:0;
    ::setsockopt(sockFd_,SOL_SOCKET,SO_KEEPALIVE,&optval,static_cast<socklen_t>(sizeof optval));
}