#ifndef EVENT_LOOP_THREAD_POOL
#define EVENT_LOOP_THREAD_POOL

#include <string>
#include <vector>
#include <memory>
#include <functional>


struct EventLoop;
struct EventLoopThread;

struct EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) {
        numThreads_ = numThreads;
    }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started() const {
        return started_;
    }
    const std::string name() const {
        return name_;
    }
private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    size_t next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

#endif