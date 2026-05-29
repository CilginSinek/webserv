#include "network/AConnection.hpp"
#include <stdexcept>

AConnection::AConnection()
{
	throw std::runtime_error("AConnection: Default constructor is not allowed");
}

AConnection::AConnection(int fd) : _fd(fd)
{
}

AConnection::AConnection(const AConnection &other) : _fd(other._fd)
{
}

AConnection &AConnection::operator=(const AConnection &other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
	}
	return *this;
}

AConnection::~AConnection()
{
}
