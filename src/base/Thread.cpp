#include <semaphore.h>
#include <Thread.h>
#include <CurrentThread.h>

std::atomic_int32_t Thread::numCreated_(0);


Thread::Thread(Threadfunc func, const std::string &name) : 
        started_(false), 
        joined_(false), 
        tid_(0), 
        func_(std::move(func)), 
        name_(name) {
            setDefaultName();
}

Thread::~Thread() {
    if(started_ && !joined_) {
        thread_ -> detach();
    }
}

void Thread::start() {
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&] {
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));


    sem_wait(&sem);
}

void Thread::join() {
    joined_ = true;
    thread_ -> join();
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if(name_.empty()) {
        char buf[22] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}