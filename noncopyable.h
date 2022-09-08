//
// Created by ball on 9/7/22.
//
#pragma once
//noncopyable被继承以后，派生类对象可以正常的构造和析构但是不可以拷贝构造和赋值
class noncopyable{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
