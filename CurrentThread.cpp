//
// Created by ball on 9/9/22.
//

#include "CurrentThread.h"

namespace CurrentThread {
     __thread int t_cachedTid = 0;

     void cachedTid(){
         if(t_cachedTid == 0) {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
         }
     }
}

int main(){return 0;}