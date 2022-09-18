//
// Created by ball on 9/14/22.
//

#include "functional"

#include "iostream"
#include "thread"
#include "unistd.h"

class TestCallback{
public:
    using testCallback = std::function<void(void)>;
    TestCallback(){};
    ~TestCallback(){};

    void setTestCallback(const testCallback &cb) {
        testCallback_ = std::move(cb);
    }
    void exec(){
        if (testCallback_) {
            testCallback_();
        }
    }
private:
    testCallback testCallback_;
};

class Callback{
public:
    Callback():testCallback_(new TestCallback()){
        testCallback_->setTestCallback(std::bind(&Callback::incr,this));
        testCallback_->exec();
    }
    void incr(void) {
        std::cout<<"incr"<<std::endl;
        return ;
    }

private:
    TestCallback* testCallback_;
};

int sum = 0;
void Thread01(int id) {
    while(id++ < 1000)
        asm volatile(" add $1,%0": "+m"(sum));
        //        sum++;
}

int main(){
    std::thread t1(Thread01,0);
    std::thread t2(Thread01,0);
    t1.join();
    t2.join();

    std::cout<<"main sum ="<<sum<<"\n";
    return 0;

}