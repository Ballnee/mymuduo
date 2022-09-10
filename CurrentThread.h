//
// Created by ball on 9/9/22.
//

#ifndef MYMUDUO_CURRENTTHREAD_H
#define MYMUDUO_CURRENTTHREAD_H
#include "unistd.h"
#include "sys/syscall.h"

namespace CurrentThread {
    extern __thread int t_cachedTid;

    void cachedTid();
    inline int tid(){
        if(__builtin_expect(t_cachedTid == 0,0)){
            cachedTid();
        }
        return t_cachedTid;
    }
}


#endif //MYMUDUO_CURRENTTHREAD_H
