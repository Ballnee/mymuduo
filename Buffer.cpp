//
// Created by ball on 9/10/22.
//

#include "Buffer.h"
#include "sys/uio.h"
#include "unistd.h"
//从fd上读取数据   poller工作在LT模式
//缓冲区的大小是有限的，但是读取的时候并不知道这个fd的数据量是多大
ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extraBuf[65536] = {0};//alloc mem at stack 64k
    // 分配两个缓冲区
    struct iovec vec[2];
    const size_t writable  = writableBytes();
    //first buffer
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;
    //second buffer
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof extraBuf;
    //when there is enough space in this buffer, do not read into extrabuf
    const int iovcnt = writable < sizeof extraBuf ? 2:1;
//    从内核缓冲区去读
    const ssize_t n = ::readv(fd,vec,iovcnt);
    if (n<0) {
        *saveErrno = errno;
    } else if (n<writable){
        writerIndex_ += n;
    }else {  //extrabuf也存了数据
        writerIndex_ = buffer_.size();
        append(extraBuf,n-writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
    ssize_t n = ::write(fd,peek(),readableBytes());
    if (n < 0) {
        *saveErrno = errno;
    }
    return n;
}