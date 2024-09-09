#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include <noncopyable.h>
#include <Timestamp.h>
#include <Logging.h>

//前向声明
struct EventLoop;
// struct Timestamp;

struct Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    //处理Epoll返回事件的回调函数
    void handleEvent(Timestamp receiveTime);
    //设置回调函数
    void setReadCallback(ReadEventCallback cb) {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb) {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb) {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb) {
        errorCallback_ = std::move(cb);
    }

    void tie(const std::shared_ptr<void> &);

    int fd() const {
        return fd_;
    }
    int events() const {
        return events_;
    }
    //设置Epoll返回时发生的事件
    void set_revents(int revt) {
        revents_ = revt;
    }

    //通过封装了epoll_ctl的update()设置fd感兴趣的事件
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    void disableReading() {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting() {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll() {
        events_ &= kNoneEvent;
        update();
    }

    //返回fd感兴趣事件状态
    bool isNoneEvent() const {
        return events_ == kNoneEvent;
    }
    bool isReading() const {
        return events_ & kReadEvent;
    }
    bool isWriting() const {
        return events_ & kWriteEvent;
    }

    int index() {
        return index_;
    }

    void set_index(int idx) {
        index_ = idx;
    }

    EventLoop *ownerLoop() {
        return loop_;
    }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;   //该Channel所属的EventLoop
    const int fd_;      //Epoll监听的对象
    int events_;        //该fd感兴趣的事件
    int revents_;       //Epoll返回的事件
    int index_;;        //在Epoll上注册的情况

    std::weak_ptr<void> tie_;   //弱指针指向Tcpconnection(必要时候升级为shared_ptr，避免误删)
    bool tied_;                 //标志此Channel是否被调用过Channel::tie方法


    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};

#endif