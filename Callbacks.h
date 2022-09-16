//
// Created by ball on 9/10/22.
//

#ifndef MYMUDUO_CALLBACKS_H
#define MYMUDUO_CALLBACKS_H
#include "memory"
#include "functional"
#include "TimeStamp.h"

class Buffer;
class TcpConnection;

using TimerCallback = std::function<void()>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&,Buffer*,TimeStamp)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&,size_t)>;
#endif //MYMUDUO_CALLBACKS_H
