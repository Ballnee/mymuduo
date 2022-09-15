//
// Created by ball on 9/10/22.
//

#include "vector"
#include "noncopyable.h"
#include "string"
#ifndef MYMUDUO_BUFFER_H
#define MYMUDUO_BUFFER_H

/*
 * +-----------------------------------------------------------+
 * |                  |                |                       |
 * | prependabele     | readable bytes | writable bytes        |
 * | bytes            |                |                       |
 * +------------------|----------------|-----------------------|--+
 * |                  |                |                       |
 *                 readerIndex  <=   writerIndex     <=      size
 * */




class Buffer : noncopyable{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize):
                buffer_(kCheapPrepend+kInitialSize),
                readerIndex_(kCheapPrepend),
                writerIndex_(kCheapPrepend){}
    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }
    size_t writableBytes() const {
        return buffer_.size() - writerIndex_;
    }
    size_t prependableBytes() const{
        return readerIndex_;
    }
    //通过fd读取数据
    ssize_t readFd(int fd,int* saveErrno) ;
    //通过fd发送数据
    ssize_t writeFd(int fd,int* saveErrno);

    void retrieve(size_t len) {
        if (len  < readableBytes()) {
            readerIndex_ += len;
        }else {
            retrieveAll();
        }
    }
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }
    //把[data,data+len]内存中的数据追加在缓冲中
    void append(const char *data,size_t len) {
        ensureWriteableBytes(len);
        std::copy(data,data+len,beginWrite());
        writerIndex_ += len;
    }
    void retrieveAll(){
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //把onMessage函数上报的数据转成string返回
    std::string retrieveAsString(size_t len){
        std::string result(peek(),len);
        retrieve(len);
        return result;
    }
private:
    //返回可读事件的起始地址
    const char* peek() const {
        return begin() + readerIndex_;
    }

    char* begin() {
        //底层首元素地址
        return &*buffer_.begin();
    }
    const char* begin() const {
        return &*buffer_.begin();
    }

    void ensureWriteableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    char* beginWrite(){
        return begin()+writerIndex_;
    }

    const char * beginWrite() const {
        return begin() + writerIndex_;
    }


    void makeSpace(size_t len){
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_+len);
        }else {
            size_t readable = readableBytes();
            std::copy(begin()+readerIndex_,begin()+writerIndex_,begin()+kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};
#endif //MYMUDUO_BUFFER_H
