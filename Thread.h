//
// Created by ball on 9/10/22.
//

#ifndef MYMUDUO_THREAD_H
#define MYMUDUO_THREAD_H

#include "atomic"
#include "functional"
#include "noncopyable.h"
#include "thread"
#include "memory"
#include "string"
#include <unistd.h>

class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    //std::string（） 相当于无参构造
    explicit Thread(ThreadFunc,const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const {return started_;}
    pid_t tid() const {return tid_;}
    const std::string &name() const {return  name_;}
    static int numCreated(){return numCreated_;}
private:
    void setDefaultName();
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;

    static std::atomic<int> numCreated_;
};





#endif //MYMUDUO_THREAD_H
