#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

#include <Eventloop.h>
#include <EventLoopThreadPool.h>
#include <Acceptor.h>
#include <InetAddress.h>
#include <noncopyable.h>
#include <Callback.h>
#include <TcpConnection.h>


struct TcpServer : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option {
        kNoReuseport, 
        kReuseport
    };

    TcpServer(EventLoop *loop, const InetAddress &ListenAddr, const std::string &nameArg, Option option = kNoReuseport);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) {
        threadInitCallback_ = cb;
    }
    void setConnectionCallback(const ConnectionCallback &cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback &cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
        writeCompleteCallback_ = cb;
    }

    void setThreadNum(int numThreads);

    void start();

    EventLoop *getLoop() const { return loop_; }
    const std::string name() { return name_; }
    const std::string ipPort() const { return ipPort_; }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_;                       //该服务所属的EventLoop
    const std::string ipPort_;              //该服务对应的ip地址及端口号
    const std::string name_;                //该服务的名称
    std::unique_ptr<Acceptor> acceptor_;    //该服务的acceptor

    std::shared_ptr<EventLoopThreadPool> threadPool_;       //该服务的线程池

    ConnectionCallback connectionCallback_;             //创建新连接的回调函数
    MessageCallback messageCallback_;                   //针对读写消息的回调函数
    WriteCompleteCallback writeCompleteCallback_;       //针对消息发送完成的回调函数

    ThreadInitCallback threadInitCallback_;             //loop线程初始化的回调函数
    std::atomic_int started_;                           //表示该服务是否启动

    int nextConnId_;                        //连接索引
    ConnectionMap connections_;             //该服务的所有连接
};

#endif