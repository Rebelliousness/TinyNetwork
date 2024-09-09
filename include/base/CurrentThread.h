#ifndef CURRENT_THREAD_H
#define CURRENT_THREAD_H

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {
    extern __thread int t_cachedTid; //用于保存tid的缓冲，避免每次查询线程tid都陷入内核
    void cacheTid();

    inline int tid() {
        if(__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }

        return t_cachedTid;
    }

}

#endif