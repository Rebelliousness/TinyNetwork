#include <ThreadPool.h>

ThreadPool::ThreadPool (const std::string & name) 
                    : mutex_()
                    , cond_()
                    , name_(name)
                    , running_(false) {}

ThreadPool::~ThreadPool() {
    stop();
    for(const auto &t : threads_) {
        t -> join();
    }
}

void ThreadPool::start() {
    running_ = false;
    threads_.reserve(threadSize_);
    for(int i = 0;i < threadSize_;++i) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), name_ + id));
        threads_[i] -> start();
    }
    if(threadSize_ == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    cond_.notify_all();
}

size_t ThreadPool::queueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void ThreadPool::add(ThreadFunction threadFunction) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(threadFunction);
    cond_.notify_one();
}

void ThreadPool::runInThread() {
    try {
        if(threadInitCallback_) {
            threadInitCallback_();
        }

        ThreadFunction task;
        while(true) {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while(queue_.empty()) {
                    if(!running_) {
                        return;
                    }
                    cond_.wait(lock);
                }
                task = queue_.front();
                queue_.pop_front();
            }
            if(task != nullptr) {
                task();
            }
        }
    }
    catch(...) {
        LOG_WARN << "runInThread throw exception";
    }
}