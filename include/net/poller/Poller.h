#ifndef POLLER_H
#define POLLER_H

#include <noncopyable.h>
#include <Channel.h>
#include <Timestamp.h>

#include <vector>
#include <unordered_map>

struct Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    //存储socketfd —> Channel *的映射
    using ChannelMap = std::unordered_map<int, Channel *>;

    ChannelMap channels_;
private:
    EventLoop *ownerLoop_;
};

#endif