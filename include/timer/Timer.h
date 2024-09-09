#ifndef TIMER_H
#define TIMER_H

#include <noncopyable.h>
#include <Timestamp.h>
#include <functional>

/*
    Timer类用于描述一个定时器，该定时器回调函数会在下一次超时时触发，重置定时器
*/
struct Timer : noncopyable {
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval)
        : callbacll_(std::move(cb))
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval > 0.0) 
    {
    }

    void run() const {
        callbacll_();
    }

    Timestamp expiration() const {
        return expiration_;
    }
    bool repeat() const {
        return repeat_;
    }

    void restart(Timestamp now);
private:
    const TimerCallback callbacll_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
};


#endif