#ifndef EPOLL_POLLER_H
#define EPOLL_POLLER_H

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

#include <Logging.h>
#include <Poller.h>
#include <Timestamp.h>


struct EpollPoller : public Poller {
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
    
private:
    using EventList = std::vector<epoll_event>;

    //默认监听事件的数量
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList *activaeChannels) const;
    void update(int operation, Channel *channel);

    int epollfd_;
    EventList events_;
};

#endif