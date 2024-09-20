#include <functional>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>

#include <TcpConnection.h>
#include <Logging.h>
#include <Socket.h>
#include <Channel.h>
#include <Eventloop.h>

static EventLoop *CheckLoopNotNull(EventLoop *loop) {
    /**
     * 检查EventLoop是否有效，若无效则出错
     */
    if(loop == nullptr) {
        LOG_FATAL << "mainLoop is nullptr";
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop
                            , const std::string &nameArg
                            , int sockfd
                            , const InetAddress &localAddr
                            , const InetAddress &peerAddr)
            : loop_(loop)
            , name_(nameArg)
            , state_(kConnecting)
            , reading_(true)
            , socket_(new Socket(sockfd))
            , channel_(new Channel(loop, sockfd))
            , localAddr_(localAddr)
            , peerAddr_(peerAddr)
            , highWaterMark_(64 * 1024 * 1024)
{
    channel_ -> setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_ -> setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_ -> setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_ -> setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO << "TcpConnection::ctor[" << name_.c_str() << "] at fd = " << sockfd;
    socket_ -> setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO << "TcpConnection::dtor[" << name_.c_str() << "] at fd = " << channel_ -> fd() << "state = " << static_cast<int>(state_);
}

//发送数据
void TcpConnection::send(const std::string &buf) {
    if(state_ == kConnected) {
        if(loop_ -> isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        }
        else {
            void(TcpConnection::*fp)(const void *data, size_t len) = &TcpConnection::sendInLoop;
            loop_ -> runInLoop(std::bind(fp, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::send(Buffer *buf) {
    if(state_ == kConnected) {
        if(loop_ -> isInLoopThread()) {
            sendInLoop(buf -> peek(), buf -> readableBytes());
            buf -> retrieveAll();
        }
        else {
            void(TcpConnection::*fp)(const std::string &message) = &TcpConnection::sendInLoop;
            loop_ -> runInLoop(std::bind(fp, this, buf -> retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == kDisconnected) {
        LOG_ERROR << "disconnected, stop writing";
        return;
    }

    if(!channel_ -> isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_ -> fd(), data, len);
        if(nwrote >= 0) {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_) {
                loop_ -> queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else {
            nwrote = 0;
            if(errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::sendInLoop";
                if(errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if(!faultError && remaining > 0) {
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_ && oldlen < highWaterMark_ &&highWaterMarkCallback_) {
            loop_ -> queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining));
        }
        outputBuffer_.append((char *)data + nwrote, remaining);
        if(!channel_ -> isWriting()) {
            channel_ -> enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if(state_ == kConnected) {
        setState(kDisconnecting);
        loop_ -> runInLoop(std::bind(&TcpConnection::shutdownInLoop, this)); 
    }
}

void TcpConnection::shutdownInLoop() {
    if(!channel_ -> isWriting()) {
        socket_ -> shutdownWrite();
    }
}

void TcpConnection::connectionEstablished() {
    setState(kConnected);

    channel_ -> tie(shared_from_this());
    channel_ -> enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestrooyed() {
    if(state_ == kConnected) {
        setState(kDisconnected);
        channel_ -> disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_ -> remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_ -> fd(), &savedErrno);
    if(n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if(n == 0) {
        handleClose();
    }
    else {
        errno = savedErrno;
        LOG_ERROR << "TcpConnection::handleRead() failed";
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if(channel_ -> isWriting()) {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_ -> fd(), &saveErrno);
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_ -> disableWriting();
                if(writeCompleteCallback_) {
                    loop_ -> queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(!state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
        else {
            LOG_ERROR << "TcpConnection::handleWrite() failed";
        }
    }
    else {
        LOG_ERROR << "TcpConnection fd = " << channel_ -> fd() << " is down, no more writing";
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    channel_ -> disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;

    if(::getsockopt(channel_ -> fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen)) {
        err = errno;
    }
    else {
        err = optval;
    }
    LOG_ERROR << "TcpConnection::handleError name:" << name_.c_str() << " - SP_ERROR : " << err;
}