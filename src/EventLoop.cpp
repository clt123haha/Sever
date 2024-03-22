#include "EventLoop.h"
#include "Channel.h"

EventLoop::~EventLoop()
{
}

EventLoop::EventLoop(Epoll * _epoll) : epoll(_epoll),quit(false)
{
}

EventLoop::EventLoop():quit(false)
{
	epoll = new Epoll();
}

void EventLoop::loop()
{
    while (!quit) {
        std::vector<Channel*> chs;
        chs = epoll->poll();
        for (auto it = chs.begin(); it != chs.end(); ++it) {
            (*it)->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel * channel)
{
	epoll->updateChannel(channel);
}
