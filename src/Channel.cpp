#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* _loop, int _fd):loop(_loop),fd(_fd), events(0),ready(0),inEpoll(false)
{
}

Channel::~Channel()
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
	inEpoll = false;
}

void Channel::handleEvent()
{
	if (ready & (EPOLLIN | EPOLLPRI)) {
		readCallback();
	}
	if (ready & (EPOLLOUT)) {
		writeCallback();
	}
}

void Channel::enableRead()
{
	events |= EPOLLIN | EPOLLPRI;
	loop->updateChannel(this);
}

int Channel::getFd()
{
	return fd;
}

uint32_t Channel::getEvents()
{
	return events;
}

uint32_t Channel::getReady()
{
	return ready;
}

bool Channel::getInEpoll()
{
	return inEpoll;
}

void Channel::setInEpoll(bool _in)
{
	inEpoll = _in;
}

void Channel::useET()
{
	events |= EPOLLET;
	loop->updateChannel(this);
}

void Channel::setReady(uint32_t _ev) {
	ready = _ev;
}

void Channel::setReadCallback(std::function<void()> _callback)
{
	readCallback = move(_callback);
}

void Channel::setWriteCallback(std::function<void()> _callback)
{
	writeCallback = move(_callback);
}
