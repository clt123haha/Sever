#pragma once
#include "Socket.h"
#include "Epoll.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <map>
#include <malloc.h>
class Server
{
public:
	Server(EventLoop*);
	~Server();
	void getNewConnect(Socket*);
	void DeleteConnection(int);
private:
	ThreadPool* thread_pool_;
	std::map<int, Connection*> connections;
	vector<EventLoop*> sub_reactors_;
	function<void(Connection *)> new_connect_callback_;
	Channel* channel;
	Acceptor* acceptor;
	EventLoop* loop;
};

