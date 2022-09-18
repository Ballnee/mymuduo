//
// Created by ball on 9/15/22.
//

#include "mymuduo/TcpServer.h"
#include "mymuduo/EventLoop.h"
#include "mymuduo/TimerId.h"
#include "mymuduo/EventLoopThread.h"
#include "mymuduo/Logger.h"
#include "mymuduo/Channel.h"
#include "mymuduo/CurrentThread.h"


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

namespace test03 {

#include <stdio.h>
#include <unistd.h>

    int cnt = 0;
    EventLoop* g_loop;

    void printTid()
    {
        printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
        printf("now %s\n", TimeStamp::now().toString().c_str());
    }

    void print(const char* msg)
    {
        printf("msg %s %s\n", TimeStamp::now().toString().c_str(), msg);
        if (++cnt == 20)
        {
            g_loop->quit();
        }
    }

    void cancel(TimerId timer)
    {
        g_loop->cancel(timer);
        printf("cancelled at %s\n", TimeStamp::now().toString().c_str());
    }

    int test03()
    {
        printTid();
        sleep(1);
        {
            EventLoop loop;
            g_loop = &loop;

            print("main");
            loop.runAfter(1, std::bind(print, "once1"));
            loop.runAfter(1.5, std::bind(print, "once1.5"));
            loop.runAfter(2.5, std::bind(print, "once2.5"));
            loop.runAfter(3.5, std::bind(print, "once3.5"));
            TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
            loop.runAfter(4.2, std::bind(cancel, t45));
            loop.runAfter(4.8, std::bind(cancel, t45));
            loop.runEvery(2, std::bind(print, "every2"));
            TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
            loop.runAfter(9.001, std::bind(cancel, t3));

            loop.loop();
            print("main loop exits");
        }
        sleep(1);
        {
            EventLoopThread loopThread;
            EventLoop* loop = loopThread.startLoop();
            loop->runAfter(2, printTid);
            sleep(3);
            print("thread loop exits");
        }
    }
}


int main(){

//    test01::test01();
//    test02::test02();
        test03::test03();
}
