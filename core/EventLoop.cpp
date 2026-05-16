#include "EventLoop.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdio>

EventLoop::EventLoop(/* args */)
{
	epollFd =  epoll_create(1);
}

EventLoop::~EventLoop()
{
}

void	EventLoop::run()
{
	std::vector <ServerSocket *>::iterator it;
	struct epoll_event *events;
	bool				isServer;
	int					event_count;

	isServer = false;
	while (true)
	{
		event_count = (epoll_wait(epollFd, events, INT32_MAX, 100));

		for (size_t i = 0; i < event_count; i++)
		{
			for (it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
			{
				if ((*it)->getFd() == events[i].data.fd)
				{
					isServer = true;
					break;
				}
			}
			if (isServer)
				handleServerSocket(events[i].data.fd);
			else
				handleClientEvent(events[i].data.fd, events[i].events); //EPOLLIN  EPOLLOUT EPOLLER EPOLLHUP
		}		
	}
}

void	EventLoop::handleServerSocket(int fd)
{
	connections[fd]->getFd();
}

void	EventLoop::handleClientEvent(int fd, u_int32_t events)
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

void	EventLoop::addConnection(AConnection connec, u_int32_t events)
{

}

void	EventLoop::modifyConnection(int fd, u_int32_t events) {}
void	EventLoop::removeConnection(int fd) {}
