//
// Created by ball on 9/7/22.
//
#pragma once

#include <iostream>
#include <string>
class TimeStamp {
public:
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch);

    static const int kMicroSecondsPerSecond = 1000 * 1000;
    static TimeStamp invalid() { return TimeStamp();}
    static TimeStamp now();
    bool valid(){microSecondsSinceEpoch_ > 0;}
    std::string toString() const;

    int64_t microSecondsSinceEpoch() {return microSecondsSinceEpoch_;}

    inline bool operator<(TimeStamp rhs)
    {
        return this->microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(TimeStamp rhs)
    {
        return this->microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }


private:
    int64_t microSecondsSinceEpoch_;
};

//int main(){
//    TimeStamp t1(10);
//    TimeStamp t2(20);
//    if (t1 < t2) {
//        std::cout<<"t1<t2\n";
//    }else {
//        std::cout<<"t1>=t2\n";
//    }
//
//}