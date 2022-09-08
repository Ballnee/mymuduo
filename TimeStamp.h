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
    static TimeStamp now();
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};
