#include "EventLoop.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdio>

EventLoop::EventLoop(/* args */)
{
}

EventLoop::~EventLoop()
{
}


void 	EventLoop::addServerSocket(ServerSocket  *socket) 
{
	if (!socket)
		return;
	int fd = socket->getFd();
	if (fd == -1)
		return;
	struct epoll_event ev;	
	ev.events = EPOLLIN;
	ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        throw std::runtime_error("epoll_ctl(ADD) server socket failed");
    }
	_serverSockets.push_back(socket);
}

void	EventLoop::addConnection(AConnection connec, u_int32_t events) {}
void	EventLoop::modifyConnection(int fd, u_int32_t events) {}
void	EventLoop::removeConnection(int fd) {}
void	EventLoop::run() {}
void	EventLoop::handleServerSocket(int fd) {}
void	EventLoop::handleClientEvent(int fd, u_int32_t events) {}
