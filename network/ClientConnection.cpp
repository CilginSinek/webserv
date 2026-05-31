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
	std::string header = buffer.substr(0, buffer.find("\r\n\r\n") + 4);
	if (header.find("Transfer-Encoding: chunked") != std::string::npos)
	{
		size_t bodyEndpos = buffer.find("0\r\n\r\n", header.length());
		if (bodyEndpos == std::string::npos)
			return -1;
		return bodyEndpos + 5;
	}
	else
	{
		size_t contentLengthPos = header.find("Content-Length: ");
		if (contentLengthPos == std::string::npos)
			return header.length();
		contentLengthPos += std::string("Content-Length: ").length();
		size_t contentLengthEndPos = header.find("\r\n", contentLengthPos);
		size_t contentLength = strtoul(header.substr(contentLengthPos, contentLengthEndPos - contentLengthPos).c_str(), NULL, 10);
		if (buffer.length() >= header.length() + contentLength)
			return header.length() + contentLength;
		else
			return -1;
	}
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


Buffer ClientConnection::generateResponse(const Buffer &request) const
{
	if (request.find("Transfer-Encoding: chunked") != std::string::npos)
	{
		std::string header = request.substr(0, request.find("\r\n\r\n") + 4);
		std::string body = request.substr(request.find("\r\n\r\n") + 4);
		std::string res;
		size_t pos = 0;
		while (pos < body.length())
		{
			size_t chunkSizeEndPos = body.find("\r\n", pos);
			if (chunkSizeEndPos == std::string::npos)
				break;
			std::string chunkSizeStr = body.substr(pos, chunkSizeEndPos - pos);
			size_t chunkSize = strtoul(chunkSizeStr.c_str(), NULL, 16);
			if (chunkSize == 0)
				break;
			pos = chunkSizeEndPos + 2;
			res += body.substr(pos, chunkSize);
			pos += chunkSize + 2;
		}
		return Buffer(header + res);
	}
	else
		return Buffer(request);
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
	return generateResponse(res);
}


tConnectionState ClientConnection::getState() const
{
	return this->_state;
}

const ServerSocket* ClientConnection::getServerSocket() const
{
	return this->_serverSocket;
}