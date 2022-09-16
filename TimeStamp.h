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
    std::string toString() const;

    int64_t microSecondsSinceEpoch() {return microSecondsSinceEpoch_;}



private:
    int64_t microSecondsSinceEpoch_;
};
