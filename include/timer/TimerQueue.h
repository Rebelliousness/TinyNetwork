#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H

#include <Timestamp.h>
#include <Channel.h>

#include <vector>
#include <set>

struct EventLoop;
struct Timer;

struct TimerQueue {
public:
    using TimerCallback = std::function<void()>;

    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    void addTimer(TimerCallback cb, Timestamp when, double interval);
private:
    using Entry = std::pair<Timestamp, Timer *>;
    using TimerList = std::set<Entry>;

    void addTimerInLoop(Timer *timer);

    void handleRead();

    void resetTimerfd(int timerfd_, Timestamp expiration);

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry> &expired, Timestamp now);

    bool insert(Timer *timer);

    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
    bool callingExpiredTimers_;
};

#endif