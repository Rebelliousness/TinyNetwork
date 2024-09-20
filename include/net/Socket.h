#ifndef SOCKET_H
#define SOCKET_H

#include <noncopyable.h>

struct InetAddress;

struct Socket : noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const {
        return sockfd_;
    }

    void bindAddress(const InetAddress &localaddr);

    void listen();
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);    //设置Nagel算法
    void setReuseAddr(bool on);     //设置地址复用
    void setReusePort(bool on);     //设置端口复用
    void setKeepAlive(bool on);     //设置长链接
private:
    const int sockfd_;
};

#endif