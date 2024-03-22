#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include<sys/types.h>
#include "util.h"
#include <strings.h>
#include <fcntl.h>
#include <errno.h>


class InetAddress
{
public:
	InetAddress();
	InetAddress(const char*, uint16_t);
	struct sockaddr_in getAddr();
	void setInetAddr(sockaddr_in);
	char* getIp();
	int getPort();
private:
	struct sockaddr_in addr;
};


class Socket
{
public:
	void setnonblocking();
	int accept(InetAddress*);
	Socket();
	Socket(int);
	~Socket();
	void bind(InetAddress*);
	int getFd();
	void listen();
private:
	int socketFd;
};

