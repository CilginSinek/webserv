#include "core/EventLoop.hpp"
#include "parser/RequestParse.hpp"
#include "parser/ResponseParse.hpp"
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
    if (epollFd != -1)
        close(epollFd);
}

void	EventLoop::run()
{
	std::vector <ServerSocket *>::iterator it;
	struct epoll_event 	events[1028];
	bool				isServer;
	int					event_count;

	isServer = false;
	while (true)
	{
		event_count = (epoll_wait(epollFd, events, 1028, 100));
		if (event_count == -1)
			continue;
		for (int i = 0; i < event_count; i++)
		{
			isServer = 0;
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
	{
		this->_connections.insert(std::make_pair(client_fd, ClientConnection(client_fd, socket)));
		fcntl(client_fd, F_SETFL, O_NONBLOCK); //client can send data to me 
		addConnection(client_fd, EPOLLIN);
	}
	
	struct epoll_event ev;
	ev.events = EPOLLIN; //inform me when client sends data
	ev.data.fd = client_fd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &ev);  //added watch list
}


//control the three main event: connection error, client http request, http response
void	EventLoop::handleClientEvent(int fd, u_int32_t events)
{
	char		buffer[1024];
	int			bytes_read;
	int			isKeepAlive = 1;

	if ((events & EPOLLERR) || (events & EPOLLHUP))
	{
		this->_connections.erase(fd);
		removeConnection(fd);
		close(fd);
		return ;
	}
	if (events & EPOLLIN)
	{
		bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			this->_connections[fd].addReadBuffer(Buffer(buffer));
			modifyConnection(fd, EPOLLOUT);
		}
		else
		{
			this->_connections.erase(fd);
			removeConnection(fd);
			close(fd);

		}
	}
	else if (events & EPOLLOUT)
	{
		while (this->_connections[fd].getState() == WRITING)
		{
			Buffer responseBuffer = this->_connections[fd].getWriteBuffer();
			RequestParse requestParse(responseBuffer);
			ResponseParse responseParse(requestParse, this->_connections[fd].getServerSocket()->getConfig());
			debugLogger("Request fd " + ft_itos(fd) + ":\n" + responseBuffer);
			responseBuffer = responseParse.generateResponse();

			debugLogger("Response fd " + ft_itos(fd) + ":\n" + responseBuffer);
			send(fd, responseBuffer.c_str(), responseBuffer.size(), 0); 
		}
		if (isKeepAlive)
		{
			modifyConnection(fd, EPOLLIN);
		}
		else
		{
			this->_connections.erase(fd);
			removeConnection(fd);
			close(fd);
		}
	}
}

void 	EventLoop::addServerSocket(ServerSocket  *socket) 
{
	if (!socket)
		return;
	int fd = socket->getFd();
	if (fd == -1)
		return;

		
    if (addConnection(fd, EPOLLIN) == -1)
    {
		throw std::runtime_error("epoll_ctl(ADD) server socket failed");
    }
	_serverSockets.push_back(socket);
}

// WRAPPER FUNCTIONS

int	EventLoop::addConnection(int fd, u_int32_t events)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl (ADD) failed for fd: " << fd << std::endl;
		return (-1);
	}
	return (1);
}

void	EventLoop::modifyConnection(int fd, u_int32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
	    std::cerr << "epoll_ctl (MOD) failed for fd: " << fd << std::endl;
}

void	EventLoop::removeConnection(int fd)
{
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        std::cerr << "epoll_ctl (DEL) failed for fd: " << fd << std::endl;
}
