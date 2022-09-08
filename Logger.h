//
// Created by ball on 9/7/22.
//
#pragma once


#include "noncopyable.h"
#include <string>
#include <cstdio>
//定义日志级别  INFO ERROR FATAL DEBUG

#define LOG_INFO(LogMsgFormat,...) \
    do {                           \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(INFO);            \
            char buf[1024]  = {0}; \
            snprintf(buf,1024,LogMsgFormat,##_VA_ARGS_); \
            logger.log(buf); \
    }while(0)

#define LOG_ERROR(LogMsgFormat,...) \
    do {                           \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(ERROR);            \
            char buf[1024]  = {0}; \
            snprintf(buf,1024,LogMsgFormat,##_VA_ARGS_); \
            logger.log(buf); \
    }while(0)

#define LOG_FATAL(LogMsgFormat,...) \
    do {                           \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(FATAL);            \
            char buf[1024]  = {0}; \
            snprintf(buf,1024,LogMsgFormat,##_VA_ARGS_); \
            logger.log(buf); \
    }while(0)

#ifdef MUDEBUG
#define LOG_DEBUG(LogMsgFormat,...) \
    do{                           \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(DEBUG);            \
            char buf[1024]  = {0};  \
            snprintf(buf,1024,LogMsgFormat,##_VA_ARGS_); \
            logger.log(buf); \
    }while(0)
#else
    #define LOG_DEBUG(LogMsgFormat,...)
#endif


enum LogLevel {
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};

class Logger : noncopyable {
public:
    //获取日志唯一实例对象
    static Logger& instance();
    //设置日志级别
    void setLogLevel(int level);
    void log(std::string msg);
private:
    int logLevel_;
    Logger(){}

};