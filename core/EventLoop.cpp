#include "core/EventLoop.hpp"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>

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
				handleServerSocket(*it);
			else
				handleClientEvent(events[i].data.fd, events[i].events); //EPOLLIN  EPOLLOUT EPOLLER EPOLLHUP
		}		
	}
}

void	EventLoop::handleServerSocket(ServerSocket *socket)
{
	int client_fd = socket->acceptClient();

	if (client_fd != -1)
		fcntl(client_fd, F_SETFL, O_NONBLOCK); //client can send data to me 
	
	struct epoll_event ev;
	ev.events = EPOLLIN; //inform me when client sends data
	ev.data.fd = client_fd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &ev);  //added watch list
}


//control the three main event: connection error, client http request, http response
void	EventLoop::handleClientEvent(int fd, u_int32_t events)
{
	char	buffer[1024];
	int		bytes_read;
	char	*response;
	int		isKeepAlive;

	if ((events & EPOLLERR) || (events & EPOLLHUP))
	{
		close(fd);
		removeConnection(fd);
	}
	if (events & EPOLLIN)
	{
		epoll_event ev;
		bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
		
		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			std::cout << "The incoming request: " << buffer << std::endl;
			close(fd);
		}
		if (bytes_read == 0)
			close(fd);
		//if (bytes_read < 0)
			//close(fd);
		ev.events = EPOLLOUT;
		ev.data.fd = fd;
		epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev); //mod modify r -> w
	}
	if (events & EPOLLOUT)
	{
		response = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nMerhaba Web";
		send(fd, response, std::strlen(response), 0);
		if (isKeepAlive)
		{
			//epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
			close(fd); //test için şimdilik kapattım.
		}
		else
			close(fd);
	}
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
