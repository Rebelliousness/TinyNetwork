#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <noncopyable.h>

struct Thread : noncopyable {
public:
    using Threadfunc = std::function<void()>;
    explicit Thread(Threadfunc, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const {
        return started_;
    }

    pid_t tid() const {
        return tid_;
    }

    const std::string &name() const {
        return name_;
    }

    static int numCreated() {
        return numCreated_;
    }

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    Threadfunc func_;
    std::string name_;
    static std::atomic_int32_t numCreated_;
};

#endif