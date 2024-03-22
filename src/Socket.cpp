#include "Socket.h"

void Socket::setnonblocking()
{
    fcntl(socketFd, F_SETFL, fcntl(socketFd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress* _addr)
{
    int clnt_sockfd = -1;
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t addr_len = sizeof(addr);
    if (fcntl(socketFd, F_GETFL) & O_NONBLOCK) {
        while (true) {
            clnt_sockfd = ::accept(socketFd, (sockaddr*)&addr, &addr_len);
            if (clnt_sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
                continue;
            }
            else if (clnt_sockfd == -1) {
                errif(true, "socket accept error");
            }
            else {
                break;
            }
        }
    }
    else {
        clnt_sockfd = ::accept(socketFd, (sockaddr*)&addr, &addr_len);
        errif(clnt_sockfd == -1, "socket accept error");
    }
    _addr->setInetAddr(addr);
    return clnt_sockfd;
}

Socket::Socket(): socketFd(-1)
{
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    errif(socketFd == -1, "socket creat error");
}

Socket::Socket(int _fd): socketFd(_fd)
{
    errif(socketFd == -1, "socket creat error");
}

Socket::~Socket()
{
}

void Socket::bind(InetAddress* addr)
{
    struct sockaddr_in  inetAddress = addr->getAddr();
    errif(::bind(socketFd, (sockaddr*)&inetAddress, sizeof(inetAddress)) == -1, "socket bind error");
}

int Socket::getFd()
{
    return socketFd;
}

void Socket::listen()
{
    errif(::listen(socketFd, 2) == -1, "listen error");
}

InetAddress::InetAddress()
{
}

InetAddress::InetAddress(const char* _ip, uint16_t _port)
{
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(_ip);
    addr.sin_port = htons(_port);
}

sockaddr_in InetAddress::getAddr()
{
    return addr;
}

void InetAddress::setInetAddr(sockaddr_in _addr)
{
    addr = _addr;
}

char* InetAddress::getIp()
{
    return  inet_ntoa(addr.sin_addr);
}

int InetAddress::getPort()
{
    return addr.sin_port;
}


