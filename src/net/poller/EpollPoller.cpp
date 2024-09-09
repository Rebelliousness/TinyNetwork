#include <EpollPoller.h>
#include <string.h>

const int kNew = -1;        //某个channel还未添加至Poller
const int kAdded = 1;       //某个channel已经添加至Poller
const int kDeleted = 2;     //某个channel已经从Poller删除

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if(epollfd_ < 0) {
        LOG_FATAL << "epoll_create() error:" << errno;
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    size_t numEvents = ::epoll_wait(epollfd_, &(*events_.begin()), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    //事件发生
    if(numEvents > 0) {
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0) {
        LOG_DEBUG << "timeout!";
    }
    else {
        if(saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR << "EpollPoller::poll() failed";
        }
    }
    return now;
}

void EpollPoller::updateChannel(Channel *channel) {
    /*
    todo:获取channel在epoll中的状态
    */
    const int index = channel -> index();

    if(index == kNew || index == kDeleted) {
        if(index == kNew) {
            int fd = channel -> fd();
            channels_[fd] = channel;
        }
        else {
        }
        channel -> set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        if(channel -> isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel -> set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for(int i = 0;i < numEvents;++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel -> set_revents(events_[i].events);
        activeChannels -> push_back(channel);
    }
}

void EpollPoller::removeChannel(Channel *channel) {
    int fd = channel -> fd();
    channels_.erase(fd);

    int index = channel -> index();
    if(index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel -> set_index(kNew);
}

void EpollPoller::update(int operation, Channel *channel) {
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel -> fd();
    event.events = channel -> events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if(operation == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl() del error:" << errno;
        }
        else {
            LOG_FATAL << "epoll_ctl() add/mod error:" << errno;
        }
    }
}