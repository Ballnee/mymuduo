//
// Created by ball on 9/14/22.
//

#include "functional"

#include "iostream"


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


int main(){
    Callback c;

}