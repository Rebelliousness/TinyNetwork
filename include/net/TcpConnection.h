#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <memory>
#include <string>
#include <atomic>
#include <string>

#include <noncopyable.h>
#include <Callback.h>
#include <Buffer.h>
#include <Timestamp.h>
#include <InetAddress.h>

struct Channel;
struct EventLoop;
struct Socket;

struct TcpConnection : noncopyable , public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, 
                const std::string &nameArg, 
                int sockfd, 
                const InetAddress &localAddr, 
                const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop *getloop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddr() const { return localAddr_; }
    const InetAddress &peerAddr() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    //数据发送
    void send(const std::string &buf);
    void send(Buffer *buf);

    //关闭连接
    void shutdown();

    //设置自定义的回调函数
    void setConnectionCallback(const ConnectionCallback &cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback &cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
        writeCompleteCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) {
        highWaterMarkCallback_ = cb;
    }

    //TcpServer调用的函数
    void connectionEstablished();   //连接建立函数
    void connectDestrooyed();       //连接销毁函数

private:
    enum StateE {
        kDisconnected,              //已经断开连接 
        kConnecting,                //正在连接
        kConnected,                 //已连接
        kDisconnecting              //正在断开连接
    };
    void setState(StateE state) {
        state_ = state;
    }

    /**
     * 注册到channel上的回调函数，当channel有事件发生时会调用以下回调函数
     * 这些函数在调用用户注册的回调函数
     */
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string &message);
    void shutdownInLoop();

    EventLoop *loop_;                   //该连接所属EventLoop
    const std::string name_;
    std::atomic_int state_;             //连接状态
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;               //本地服务器地址
    const InetAddress peerAddr_;                //对端服务器地址

    /**
     * 用户自定义事件处理的回调函数，然后传递给TcpServer
     * TcpServer创建TcpConnection对象时会将回调函数到TcpConnection中
     */
    ConnectionCallback connectionCallback_;             //创建连接时回调函数
    MessageCallback messageCallback_;                   //有读写消息时的回调函数
    WriteCompleteCallback writeCompleteCallback_;       //消息发送完成后的回调函数
    CloseCallback closeCallback_;                       //客户端关闭连接的回调函数
    HighWaterMarkCallback highWaterMarkCallback_;       //超出水位时的回调函数
    size_t highWaterMark_;

    Buffer inputBuffer_;        //读取缓冲区
    Buffer outputBuffer_;        //写入缓冲区
};

#endif