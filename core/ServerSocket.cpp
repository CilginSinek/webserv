#include "network/ServerSocket.hpp"
#include <sys/socket.h>  // socket() setsockopt() bind() listen() accept()
#include <netinet/in.h> // sockaddr_i AF_INEY INADDR_ANY htons()
#include <arpa/inet.h> // inet_pton()
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <string>

ServerSocket::ServerSocket(ServerConfig serverConfig) : _fd(-1) , _config(serverConfig)
{
}

ServerSocket::~ServerSocket()
{
	if (_fd != -1)
	{
		::close(_fd);
	}
}


void ServerSocket::open()
{
    if (_fd != -1)
        return;

	/*  AF_INET: I use IPv4  
		SOCK_STREAM: TCP socket
		0: protocol opens automaticly */

    _fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
	/*	errno:	Permission denied
				Too many open files
				Address family not supported */
        throw std::runtime_error(std::string("socket() failed: ") + std::strerror(errno));

    int opt = 1;
	/*	it's socket settings function
		give option to socket.  SOL_SOCKET,
		SO_REUSEADDR: can use same port again after the short time. It blocks TIME_WAIT feasability. */
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        ::close(_fd);
        _fd = -1;
        throw std::runtime_error(std::string("setsockopt() failed: ") + std::strerror(errno));
    }
	/* this is a struct which store the ipv4 informations sin_family sin_port sin_addr*/
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; //ipv4

    int port =  _config.getPort(); 
    const char *host_cstr = _config.getServerIp().c_str();

	/* Turn port value into network byte order*/
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host_cstr, &addr.sin_addr) != 1)
    {
        ::close(_fd);
        _fd = -1;
        throw std::runtime_error("invalid listen address");
    }
	/*	connect socket to host/port 
		"I will listen on this IP and port."*/
    if (bind(_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1) // ?
    {
        ::close(_fd);
        _fd = -1;
        throw std::runtime_error(std::string("bind() failed: ") + std::strerror(errno));
    }

    if (listen(_fd, SOMAXCONN) == -1)
    {
        ::close(_fd);
        _fd = -1;
        throw std::runtime_error(std::string("listen() failed: ") + std::strerror(errno));
    }

    setNonBlocking(_fd);
}

int ServerSocket::acceptClient()
{
    if (_fd == -1)
        return -1;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = ::accept(_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &len);
    if (client_fd == -1)
    {
        // non-blocking accept may fail with EAGAIN/EWOULDBLOCK
        return -1;
    }

    setNonBlocking(client_fd);
    return client_fd;
}

void ServerSocket::close()
{
    if (_fd != -1)
    {
        ::close(_fd);
        _fd = -1;
    }
}

void ServerSocket::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

const ServerConfig& ServerSocket::getConfig() const
{
    return _config;
}

int ServerSocket::getFd()
{
    return (_fd);
}
