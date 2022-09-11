//
// Created by ball on 9/10/22.
//

#include "TcpConnection.h"
#include "errno.h"
#include "Logger.h"
#include "Callbacks.h"
#include "string"

static EventLoop* CheckLoopNotNuLL(EventLoop *loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n",__FILE__,__FUNCTION__ ,__LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockFd, const InetAddress &localAddr,
                             const InetAddress &peerAddr):
                             loop_(CheckLoopNotNuLL(loop)),
                             name_(name),
                             state_(kConnecting),
                             reading_(true),
                             socket_(new Socket(sockFd)),
                             channel_(new Channel(loop,sockFd)),
                             localAddr_(localAddr),
                             peerAddr_(peerAddr),
                             highWaterMark_(64*1024*1024){ //
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));
    LOG_INFO("tcpConnection::ctor[%s] at fd=%d\n",name_.c_str(),sockFd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::~TcpConnection %s at fd=%d state=%d\n",name_.c_str(),channel_->fd(),(int)state_);
}

void TcpConnection::send(const std::string& buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(),buf.size());
        }else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this,buf.c_str(),buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
        ssize_t nwrote = 0;
        size_t remaining = len;
        bool faultError = false;
        //
        if(state_ == kDisconnected) {
            LOG_ERROR("disconnected,give up writing!");
            return;
        }
        if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
            nwrote = ::write(channel_->fd(),message,len);
            if(nwrote >= 0) {
                remaining = len - nwrote;
//                既然数据发送完成，就不用给channel设置epoll out事件了
                if (remaining == 0 && writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
            }else {
                nwrote = 0;
                if (errno != EWOULDBLOCK) {
                    LOG_ERROR("TcpConn::sendInLoop");
                    if(errno == EPIPE || errno == ECONNRESET) {
                        faultError = true;
                    }
                }
            }
        }
        /*说明当前write，并没有把数据全部发送出去，剩余的数据需要保留在缓冲区中，然后给channel注册epollout事件，
            poller发现tcp的发送缓冲有空间，会通知相应的sockfd->channel，调用handleWrite回调,把缓冲区数据发送完成。
         * */
        if(!faultError && remaining > 0) {
            //目前发送缓冲区剩余待发送的数据长度
            size_t oldLen = outputBuffer_.readableBytes();
            if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_&&highWaterMarkCallback_) {
                loop_->queueInLoop(
                        std::bind(highWaterMarkCallback_,shared_from_this(),oldLen+remaining)
                        );
                outputBuffer_.append(static_cast<const char*>(message)+nwrote,remaining);
                if(!channel_->isWriting()) {
                    channel_->enableWriting(); //一定需要注册写事件  接下来的数据由epoll触发发送
                }
            }
        }

}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());//把channel对象和TcpConnection对象绑定
    channel_->enableReading();//在poller上面创建监听读事件
    //新连接建立，执行回调
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if(state_ == kConnected) {
        setState(kDisconnected);
            channel_->disableAll();  //
            connectionCallback_(shared_from_this());
    }
    channel_->remove();//把channel从poller中删除
}

void TcpConnection::shutdown() {
    if(state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(
                std::bind(&TcpConnection::shutdownInLoop, this)
                );
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {  //当前outputBuffer上的数据全部发送完成
        socket_->shutdownWrite();//关闭写端
    }
}

void TcpConnection::handleRead(TimeStamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
    if (n > 0) {
        //已连接的用户发生了可读事件 执行回调
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }else if (n == 0) {
        handleClose();
    }else {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        int savedError = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(),&savedError);
        if (n > 0 ) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if(writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
            }
            if(state_ == kDisconnecting) {
                shutdownInLoop();
            }
        }else {
            LOG_ERROR("TcpConnection::handleWrite\n");
        }
    }else {
        LOG_ERROR("tcpConnection fd=%d is down,no more writing\n",channel_->fd());
    }
}


void TcpConnection::handleClose() {
    LOG_INFO("fd=%d state=%d \n",channel_->fd(),int (state_));
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err  = 0;
    if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen) < 0) {
        err = errno;
    }else {
        err= optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n",name_.c_str(),err);

}