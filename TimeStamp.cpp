//
// Created by ball on 9/7/22.
//

#include "TimeStamp.h"
#include <time.h>
#include <cstdio>
#include <sys/time.h>
TimeStamp::TimeStamp():microSecondsSinceEpoch_(0){}

TimeStamp::TimeStamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch){}
TimeStamp TimeStamp::now(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int64_t sec = tv.tv_sec;
    return TimeStamp(sec*kMicroSecondsPerSecond + tv.tv_usec);
}
std::string TimeStamp::toString() const {
    char buf[128] = {0};
    int64_t tmp = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    tm *tm_time = localtime(&tmp);
    snprintf(buf,128,"%4d/%02d/%02d %02d:%02d:%02d"
             ,tm_time->tm_year+1900,tm_time->tm_mon+1,tm_time->tm_mday
             ,tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);
    return buf;
}


