#include "Server.h"

Server::Server(EventLoop* _loop):loop(_loop)
{
	acceptor = new Acceptor(loop);
	std::function<void(Socket*)> cb = bind(&Server::getNewConnect, this, std::placeholders::_1);
	acceptor->setCallback(cb);

	int size = static_cast<int>(10);
	thread_pool_ = new ThreadPool(size);
	for (int i = 0; i < size; ++i) {
		sub_reactors_.push_back(new EventLoop());
	}

	for (int i = 0; i < size; ++i) {
		std::function<void()> sub_loop = std::bind(&EventLoop::loop, sub_reactors_[i]);
		thread_pool_->add(std::move(sub_loop));
	}
}

Server::~Server()
{
	delete this->acceptor;
	delete this->channel;
	delete this->loop;
	delete this->thread_pool_;
	map<int, Connection*>().swap(connections);
	malloc_trim(0);
}

void Server::getNewConnect(Socket * sock)
{
	errif(sock->getFd() == -1,"client socket error");
		
	uint64_t random = sock->getFd() % sub_reactors_.size();
	Connection* conn = new Connection(sub_reactors_[random],sock);
	std::function<void(int)> cb = bind(&Server::DeleteConnection, this, std::placeholders::_1);
	conn->setDeleteConnectionCallback(cb);
	connections[sock->getFd()] = conn;
}

void Server::DeleteConnection(int fd)
{
	errif(fd == -1, "client socket error");
	uint64_t random = fd % sub_reactors_.size();
	if (connections.find(fd) != connections.end())
	{
		Connection* con = connections[fd];
		connections.erase(fd);
		delete con; // 析构自动关闭资源
		con = nullptr;
	}
}
