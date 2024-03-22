#pragma once
#include "Epoll.h"

class EventLoop
{
private:
	Epoll* epoll;
	bool quit;
public:
	~EventLoop();
	EventLoop(Epoll*);
	EventLoop();
	void loop();
	void updateChannel(Channel*);
};

