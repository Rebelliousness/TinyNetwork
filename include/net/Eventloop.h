#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <noncopyable.h>
#include <Timestamp.h>
#include <CurrentThread.h>
#include <TimerQueue.h>
#include <functional>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

struct Channel;
struct Poller;

struct EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTIme() const {
        return pollReturnTime_;
    }

    void runInLoop(Functor cb);

    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid();
    }

    void runAt(Timestamp timestamp, Functor &&cb) {
        timerQueue_ -> addTimer(std::move(cb), timestamp, 0.0);
    }

    void runAfter(double waitTime, Functor &&cb) {
        Timestamp time(addTime(Timestamp::now(), waitTime));
        runAt(time, std::move(cb));
    }

    void runEvery(double interval, Functor &&cb) {
        Timestamp time(addTime(Timestamp::now(), interval));
        timerQueue_ -> addTimer(std::move(cb), time, interval);
    }


private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel *>;
    std::atomic_bool looping_;  //原子操作，通过compare_and_swap指令进行操作
    std::atomic_bool quit_;
    std::atomic_bool callingPendingFunctors_;
    const pid_t threadId_;
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    /*
        TODO:eventfd用于线程通知机制
        当mainLoop获取一个新用户的Channel时需要通过轮询算法选择一个subLoop来进行处理
    */

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activateChannels_;
    Channel *currentActiveChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

#endif