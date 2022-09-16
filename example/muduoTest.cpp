//
// Created by ball on 9/15/22.
//

#include "mymuduo/TcpServer.h"
#include "mymuduo/EventLoop.h"
#include "mymuduo/Logger.h"
#include "mymuduo/Channel.h"


namespace test01{

EventLoop *g_loop;

void threadFunc() {
    g_loop->loop();
}

//检查一个thread只有一个loop的错误，负面测试
void test01() {
    EventLoop loop;
    g_loop = &loop;
    Thread t(threadFunc, "test");
    t.start();
    t.join();
    return;
}

}
#include <strings.h>
#include <unistd.h>
#include "sys/timerfd.h"
namespace test02 {

    EventLoop* g_loop;
    void timeout(TimeStamp t){
        std::cout<<"Timeout!\n";
//        g_loop->quit();
    }
    //将timerfd的readable事件通过channel转发给timeout函数
    void test02(){
        EventLoop loop;
        g_loop = &loop;

        int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK |TFD_CLOEXEC);
        Channel channel(&loop,timerfd);
        channel.setReadCallback(timeout);
        channel.enableReading();

        struct itimerspec howlong;
        bzero(&howlong,sizeof howlong);
        howlong.it_interval.tv_sec = 20;
        howlong.it_interval.tv_nsec = 10e6*2;
        howlong.it_value.tv_sec = 5;
        ::timerfd_settime(timerfd,0,&howlong,NULL);
        loop.loop();
        ::close(timerfd);

    }
}



int main(){

//    test01::test01();
    test02::test02();
}
