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

EventLoop::EventLoop(/* args */) : _stopSignal(NULL)
{
	epollFd = epoll_create(1);
}

EventLoop::~EventLoop()
{
	for (std::map<int, ClientConnection>::iterator it = _connections.begin(); it != _connections.end(); ++it)
		close(it->first);
	_connections.clear();
	if (epollFd != -1)
		close(epollFd);
}

void EventLoop::setStopSignal(volatile sig_atomic_t *stopSignal)
{
	_stopSignal = stopSignal;
}

void EventLoop::run()
{
	std::vector<ServerSocket *>::iterator it;
	struct epoll_event events[1028];
	bool isServer;
	int event_count;

	isServer = false;
	while (!_stopSignal || !*_stopSignal)
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
				handleClientEvent(events[i].data.fd, events[i].events); // EPOLLIN  EPOLLOUT EPOLLER EPOLLHUP
		}
	}
}

void EventLoop::handleServerSocket(ServerSocket *socket)
{
	int client_fd = socket->acceptClient();

	if (client_fd != -1)
	{
		debugLogger("Accepted new client connection: fd " + ft_itos(client_fd));
		this->_connections.insert(std::make_pair(client_fd, ClientConnection(client_fd, socket)));
		fcntl(client_fd, F_SETFL, O_NONBLOCK); // client can send data to me
		addConnection(client_fd, EPOLLIN);
	}
}

// control the three main event: connection error, client http request, http response
void EventLoop::handleClientEvent(int fd, u_int32_t events)
{
	char buffer[65536];
	int bytes_read;
	int isKeepAlive = 1;

	if ((events & EPOLLERR) || (events & EPOLLHUP))
	{
		this->_connections.erase(fd);
		removeConnection(fd);
		close(fd);
		return;
	}
	if (events & EPOLLIN)
	{
		bytes_read = recv(fd, buffer, sizeof(buffer), 0);
		if (bytes_read > 0)
		{
			if (this->_connections[fd].getState() == READING)
				this->_connections[fd].addReadBuffer(std::string(buffer, bytes_read));
			if (this->_connections[fd].getState() == HANDLING)
				this->_connections[fd].handleRead();
			if (this->_connections[fd].getState() == WRITING_HEADER || this->_connections[fd].getState() == WRITING_BODY)
				modifyConnection(fd, EPOLLOUT);
			else
				modifyConnection(fd, EPOLLIN);
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
		if (!this->_connections[fd].responseDataEmpty())
		{
			ResponseParse &res = this->_connections[fd].getCurrentResponseData();
			if (this->_connections[fd].getState() == WRITING_HEADER)
			{
				debugLogger("Responsed fd:" + ft_itos(fd) + " path: " + res.getBodyPath());
				this->_connections[fd].addWriteBuffer(res.getHeader());
				while (this->_connections[fd].getWriteBuffer().size() > 0)
				{
					ssize_t sent = send(fd, this->_connections[fd].getWriteBuffer().c_str(), this->_connections[fd].getWriteBuffer().size(), 0);
					if (sent <= 0)
					{
						this->_connections.erase(fd);
						removeConnection(fd);
						close(fd);
						return;
					}
					this->_connections[fd].setWriteBuffer(this->_connections[fd].getWriteBuffer().substr(sent));
				}
				this->_connections[fd].setState(WRITING_BODY);
			}
			if (this->_connections[fd].getState() == WRITING_BODY)
			{
				if (!res.getBodyPath().empty())
				{
					char buffer[65536];
					std::ifstream bodyFile(res.getBodyPath().c_str(), std::ios::binary);
					if (!bodyFile.is_open())
					{
						this->_connections[fd].popCurrentResponseData();
						if (this->_connections[fd].responseDataEmpty())
							this->_connections[fd].setState(READING);
						return;
					}
					if (!bodyFile.eof())
					{
						bodyFile.seekg(res.getSentSize());
						bodyFile.read(buffer, sizeof(buffer));
						std::streamsize bytesRead = bodyFile.gcount();
						if (bytesRead > 0)
						{
							ssize_t sent = send(fd, buffer, bytesRead, 0);
							if (sent <= 0)
							{
								if (res.isTemp())
									std::remove(res.getBodyPath().c_str());
								this->_connections.erase(fd);
								removeConnection(fd);
								close(fd);
								return;
							}
							res.setSentSize(res.getSentSize() + std::min(bytesRead, static_cast<std::streamsize>(sent)));
						}
					}
					if (bodyFile.eof())
					{
						debugLogger("Responsed fd:" + ft_itos(fd) + " path: " + res.getBodyPath() + "Response count: " + ft_itos(this->_connections[fd].getResponseCount()));
						this->_connections[fd].setResponseCount(this->_connections[fd].getResponseCount() + 1);
						bodyFile.close();
						if (res.isTemp())
							std::remove(res.getBodyPath().c_str());
						this->_connections[fd].popCurrentResponseData();
						if (this->_connections[fd].responseDataEmpty())
							this->_connections[fd].setState(READING);
					}
					else{
						modifyConnection(fd, EPOLLOUT);
						return ;
					}
				}
				else if (res.hasBody() && !res.getBodyContent().empty())
				{
					if (res.hasBody())
					{
						std::string bodyContent = res.getBodyContent();
						while (bodyContent.size() > 0)
						{
							ssize_t sent = send(fd, bodyContent.c_str(), bodyContent.size(), 0);
							if (sent <= 0)
							{
								this->_connections.erase(fd);
								removeConnection(fd);
								close(fd);
								return;
							}
							bodyContent = bodyContent.substr(sent);
						}
					}
					this->_connections[fd].popCurrentResponseData();
					if (this->_connections[fd].responseDataEmpty())
						this->_connections[fd].setState(READING);
				}
				else
				{
					this->_connections[fd].popCurrentResponseData();
					if (this->_connections[fd].responseDataEmpty())
						this->_connections[fd].setState(READING);
				}
			}
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

void EventLoop::addServerSocket(ServerSocket *socket)
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

int EventLoop::addConnection(int fd, u_int32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl (ADD) failed for fd: " << fd << std::endl;
		return (-1);
	}
	return (1);
}

void EventLoop::modifyConnection(int fd, u_int32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
		std::cerr << "epoll_ctl (MOD) failed for fd: " << fd << std::endl;
}

void EventLoop::removeConnection(int fd)
{
	if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
		std::cerr << "epoll_ctl (DEL) failed for fd: " << fd << std::endl;
}
