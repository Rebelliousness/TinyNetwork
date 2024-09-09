#include <Channel.h>
#include <Eventloop.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , tied_(false) {}


Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    loop_ -> updateChannel(this);
}

void Channel::remove() {
    loop_ -> removeChannel(this);
}


void Channel::handleEvent(Timestamp receiveTime) {
    /*
    调用Channel::tie会设置tid_ = true;
    */
    if(tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if(guard) {
            handleEventWithGuard(receiveTime);
        }
    }
    else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    
    //关闭连接事件
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if(closeCallback_) {
            closeCallback_();
        }
    }
    //读事件
    if(revents_ & (EPOLLIN | EPOLLPRI)) {
        LOG_DEBUG << "channel have read events, the fd = " << this -> fd();
        if(readCallback_) {
            LOG_DEBUG << "channel call the readCallback_(), the fd = " << this -> fd();
            readCallback_(receiveTime);
        }
    }
    //写事件
    if(revents_ & EPOLLOUT) {
        if(writeCallback_) {
            writeCallback_();
        }
    }
}