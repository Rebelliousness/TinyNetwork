#include <Eventloop.h>
#include <Logging.h>
#include <Poller.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>

__thread EventLoop *t_loopInThisThread = nullptr;

//默认的Poller IO复用接口的超时事件
const int kPollTimeMs = 10000;

int creatEventfd() {
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evfd < 0) {
        LOG_FATAL << "eventfd error: " << errno;
    }
    return evfd;
}

EventLoop::EventLoop() : looping_(false)
                        , quit_(false)
                        , callingPendingFunctors_(false)
                        , threadId_(CurrentThread::tid())
                        , poller_(Poller::newDefaultPoller(this))
                        , timerQueue_(new TimerQueue(this))
                        , wakeupFd_(creatEventfd())
                        , wakeupChannel_(new Channel(this, wakeupFd_))
                        , currentActiveChannel_(nullptr)
{
    LOG_DEBUG << "EventLoop created" << this << "the index is" << threadId_;
    LOG_DEBUG << "EventLoop created wakeupFd " << wakeupChannel_ -> fd();
    if(t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop" << t_loopInThisThread << " exists in this thread " << threadId_;
    }
    else {
        t_loopInThisThread = this;
    }

    wakeupChannel_ -> setReadCallback(std::bind(&EventLoop::handleRead, this));

    wakeupChannel_ -> enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_ -> disableAll();

    wakeupChannel_ -> remove();

    ::close(wakeupFd_);

    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    LOG_INFO << "EventLoop " << this << " start looping";

    while(!quit_) {
        // 清空activeChannels_
        activateChannels_.clear();

        pollReturnTime_ = poller_ -> poll(kPollTimeMs, &activateChannels_);
        for(Channel * channel : activateChannels_) {
            channel -> handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop事件循环需要处理的回调操作
        /*
            IO thread:mainLoop accept fd 打包成 chennel 分发给 subLoop
            mainLoop实现注册一个回调，交给subLoop来执行，wakeup subLoop 之后，让其执行注册的回调操作
            这些回调函数在 std::vector<Functor> pendingFunctors_; 之中
        */
        doPendingFunctors();
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;

    if(isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if(n!= sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << "bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_ -> updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_ -> removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    poller_ -> hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor &functor : functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}