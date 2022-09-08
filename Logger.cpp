//
// Created by ball on 9/7/22.
//

#include "Logger.h"
#include "TimeStamp.h"
Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level) {
    logLevel_ = level;
    return;
}

//写日志 {日志级别} time ：msg
void Logger::log(std::string msg) {
    switch (logLevel_) {
        case INFO:
            std::cout<<"[INFO]";
            break;
        case ERROR:
            std::cout<<"[ERROR]";
            break;
        case FATAL:
            std::cout<<"[FATAL]";
            break;
        case DEBUG:
            std::cout<<"[DEBUG]";
            break;
        default:
            break;
    }
    std::cout<<TimeStamp::now().toString()<<":"<<msg<<std::endl;
}