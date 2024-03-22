#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* _loop):loop(_loop)
{
	socket = new Socket();
	InetAddress addr("127.0.0.1", 1234);
	socket->bind(&addr);
	socket->listen();

	channel = new Channel(loop, socket->getFd());

	function<void()> cb = bind(&Acceptor::newConnect, this);
	channel->setReadCallback(cb);
	channel->enableRead();
}

Acceptor::~Acceptor()
{
	delete socket;
	delete channel;
	delete loop;
}

void Acceptor::setCallback(std::function<void(Socket*)> _callback)
{
	callback = move(_callback);
	
}

void Acceptor::newConnect()
{
	InetAddress* clnt_addr = new InetAddress();
	Socket* clnt_sock = new Socket(socket->accept(clnt_addr));
	printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), clnt_addr->getIp(), clnt_addr->getPort());
	clnt_sock->setnonblocking();  //新接受到的连接设置为非阻塞式
	callback(clnt_sock);
	delete clnt_addr;
}
