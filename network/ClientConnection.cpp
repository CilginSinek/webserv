#include "network/ClientConnection.hpp"
#include "utils/Utils.hpp"


ClientConnection::ClientConnection(): AConnection(-1)
{
	throw std::runtime_error("ClientConnection: Default constructor is not allowed");
}

ClientConnection::ClientConnection(int fd, ServerSocket *serverSocket) : AConnection(fd), _readBuffer(""), _writeBuffer(""), _serverSocket(serverSocket), _state(READING), _lastActiveTime(time(NULL))
{
}

ClientConnection::ClientConnection(const ClientConnection &other): AConnection(other)
{
	*this = other;
}

ClientConnection &ClientConnection::operator=(const ClientConnection &other)
{
	if (this != &other)
	{
		AConnection::operator=(other);
		this->_readBuffer = other._readBuffer;
		this->_writeBuffer = other._writeBuffer;
		this->_serverSocket = other._serverSocket;
		this->_state = other._state;
		this->_lastActiveTime = other._lastActiveTime;
	}
	return *this;
}

ClientConnection::~ClientConnection()
{
}

bool ClientConnection::isNeedToClose() const
{
	if (this->_readBuffer.empty() && this->_writeBuffer.empty())
		return true;
	time_t currentTime = time(NULL);
	if (difftime(currentTime, this->_lastActiveTime) > this->_serverSocket->getConfig().getKeepaliveTimeout()) 
		return true;
	return false;
}

ssize_t ClientConnection::getRequestSize(const Buffer &buffer) const
{
	if (buffer.find("\r\n\r\n") == std::string::npos)
		return -1;
	if (buffer.find("Content-Length: ") == std::string::npos)
		return buffer.find("\r\n\r\n") + 4;
	size_t contentLengthPos = buffer.find("Content-Length: ") + 16;
	size_t contentLengthEnd = buffer.find("\r\n", contentLengthPos);
	std::string contentLengthStr = buffer.substr(contentLengthPos, contentLengthEnd - contentLengthPos);
	size_t contentLength = ft_stoi(contentLengthStr);
	size_t headerEnd = buffer.find("\r\n\r\n") + 4;
	if (buffer.size() < headerEnd + contentLength)
		return -1;
	return headerEnd + contentLength;
}

void ClientConnection::addReadBuffer(const Buffer &buffer)
{
	this->_readBuffer += buffer;
	while (true)
	{
		ssize_t newReqSize = this->getRequestSize(this->_readBuffer);
		if (newReqSize == -1)
			break;
		this->_writeBuffer += this->_readBuffer.substr(0, newReqSize);
		this->_readBuffer.erase(0, newReqSize);

		if (this->_state == READING)
			this->_state = WRITING;
	}
	this->_lastActiveTime = time(NULL);
}


Buffer ClientConnection::getWriteBuffer()
{
	ssize_t reqSize = this->getRequestSize(this->_writeBuffer);
	if (reqSize == -1)
		return Buffer();
	Buffer res = this->_writeBuffer.substr(0, reqSize);
	this->_writeBuffer.erase(0, reqSize);
	this->_lastActiveTime = time(NULL);
	if (this->_writeBuffer.empty())
		this->_state = READING;
	return res;
}


tConnectionState ClientConnection::getState() const
{
	return this->_state;
}

const ServerSocket* ClientConnection::getServerSocket() const
{
	return this->_serverSocket;
}