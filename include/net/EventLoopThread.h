#ifndef EVENTLOOP_THREAD_H
#define EVENTLOOP_THREAD_H

#include <functional>
#include <mutex>
#include <condition_variable>
#include <noncopyable.h>
#include <Thread.h>

struct EventLoop;

struct EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), 
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();
private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

#endif