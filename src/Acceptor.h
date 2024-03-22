#pragma once
#include "EventLoop.h"
#include "util.h"
#include <functional>
#include "Socket.h"

class Acceptor
{
public:
	Acceptor(EventLoop*);
	~Acceptor();
	void setCallback(std::function<void(Socket*)> _callback);
	void newConnect();
private:
	Socket* socket;
	Channel* channel;
	std::function<void(Socket*)> callback;
	EventLoop* loop;
};


