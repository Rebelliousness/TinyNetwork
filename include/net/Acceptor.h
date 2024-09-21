#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <noncopyable.h>
#include <Socket.h>
#include <Channel.h>

struct EventLoop;
struct InetAddress;

struct Acceptor {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) {
        newConnectionCallback_ = cb;
    }

    bool listenning() const {
        return listenning_;
    }
    void listen();
private:
    void handleRead();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};

#endif