#include <Logging.h>
#include <Acceptor.h>
#include <InetAddress.h>

#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

static int createNonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0) {
        LOG_FATAL << "listen socket creat err " << errno;
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &ListenAddr, bool reuseport) : loop_(loop)
                                                                                    , acceptSocket_(createNonblocking())
                                                                                    , acceptChannel_(loop, acceptSocket_.fd())
                                                                                    , listenning_(false)
{
    LOG_DEBUG << "Acceptor create nonblocking socket, [fd = " << acceptChannel_.fd() << "]";

    acceptSocket_.setReuseAddr(reuseport);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(ListenAddr);

    /**
     * TcpServer::start()地东后，一旦有心用户连接，便会执行acceptChannel的回调函数
     */
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen() {
    //表示正在监听
    listenning_ = true;
    acceptSocket_.listen();
    //向acceptChannel注册读事件
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {

    InetAddress peerAddr;

    int connfd = acceptSocket_.accept(&peerAddr);

    if(connfd >= 0) {
        if(newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        }
        else {
            LOG_DEBUG << "no NewConnectionCallback()function";
            ::close(connfd);
        }
    }
    else {
        LOG_ERROR << "accept() failed";

        if(errno == EMFILE) {
            LOG_ERROR << "sockfd is over the limitation";
        }
    }
}