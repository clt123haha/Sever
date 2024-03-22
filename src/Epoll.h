#pragma once
#include "Channel.h"
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "util.h"

using namespace std;

#define MAX_EVENTS 1000

class Epoll
{
private:
	int epollFd;
	struct epoll_event* events;
public:
	Epoll();
	~Epoll();
	void deleteChannel(Channel* channel);
	void updateChannel(Channel* channel);
	vector<Channel*> poll(int timeout = -1);
};


