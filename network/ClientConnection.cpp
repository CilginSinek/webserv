#include "network/ClientConnection.hpp"
#include "utils/Utils.hpp"


ClientConnection::ClientConnection(): AConnection(-1)
{
	throw std::runtime_error("ClientConnection: Default constructor is not allowed");
}

ClientConnection::ClientConnection(int fd, ServerSocket *serverSocket) : AConnection(fd), _readBuffer(""), _writeBuffer(""), _serverSocket(serverSocket), _state(READING), _lastActiveTime(time(NULL))
{
	this->responseCount = 0;
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
		this->responseCount = other.responseCount;
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
	if (buffer.compare(0, header.size(), header) != 0)
		return -1;
	else
		return header.length();
}

size_t ClientConnection::getRequestBodySize(const Buffer &buffer) const
{
	size_t contentLengthPos = buffer.find("Content-Length: ");
	if (contentLengthPos == std::string::npos)
		return 0;
	Buffer contentLengthStr = buffer.substr(contentLengthPos + 16);
	size_t endPos = contentLengthStr.find("\r\n");
	if (endPos == std::string::npos)
		return 0;
	contentLengthStr = contentLengthStr.substr(0, endPos);
	return ft_stoi(contentLengthStr);
}

void ClientConnection::addReadBuffer(const Buffer &buffer)
{
	this->_readBuffer.append(buffer);
	while (true)
	{
		if (!this->_requestDataList.empty() && !this->_requestDataList.back().complete)
		{
			if (!this->_requestDataList.back().isChunked && this->_requestDataList.back().bodySize > 0 && this->_requestDataList.back().readedBodySize < this->_requestDataList.back().bodySize)
			{
				if (this->_requestDataList.back().bodyFilePath.empty())
				{
					std::string tempFilePath = "./temp/" + ft_itos(this->_fd) + "_" + ft_itos(time(NULL));
					std::ofstream tempFile(tempFilePath.c_str(), std::ios::binary);
					if (!tempFile.is_open())
					{
						throw std::runtime_error("Failed to create temporary file for request body");
					}
					this->_requestDataList.back().bodyFilePath = tempFilePath;
				}
				std::ofstream tempFile(this->_requestDataList.back().bodyFilePath.c_str(), std::ios::binary | std::ios::app);
				if (!tempFile.is_open())
				{
					throw std::runtime_error("Failed to open temporary file for request body");
				}
				size_t remainingBodySize = this->_requestDataList.back().bodySize - this->_requestDataList.back().readedBodySize;
				size_t toWriteSize = std::min(remainingBodySize, this->_readBuffer.size());
				tempFile.write(this->_readBuffer.c_str(), toWriteSize);
				this->_requestDataList.back().readedBodySize += toWriteSize;
				this->_readBuffer.erase(0, toWriteSize);
				if (this->_requestDataList.back().readedBodySize >= this->_requestDataList.back().bodySize)
				{
					this->_requestDataList.back().complete = true;
				}
				else
				{
					break;
				}
			}
			else
			{
				if (this->_requestDataList.back().isChunked && this->_requestDataList.back().complete == false)
				{
					if (this->_requestDataList.back().bodyFilePath.empty())
					{
						std::string tempFilePath = "./temp/" + ft_itos(this->_fd) + "_" + ft_itos(time(NULL));
						std::ofstream tempFile(tempFilePath.c_str(), std::ios::binary);
						if (!tempFile.is_open())
						{
							throw std::runtime_error("Failed to create temporary file for request body");
						}
						this->_requestDataList.back().bodyFilePath = tempFilePath;
					}
					std::ofstream tempFile(this->_requestDataList.back().bodyFilePath.c_str(), std::ios::binary | std::ios::app);
					if (!tempFile.is_open())
					{
						throw std::runtime_error("Failed to open temporary file for request body");
					}
					size_t totalwritedSize = 0;
					size_t consumed = 0;
					while (true)
					{
						size_t chunkedPos = this->_readBuffer.find("\r\n", consumed);
						if (chunkedPos == std::string::npos)
							break;
						std::string chunkSizeStr = this->_readBuffer.substr(consumed, chunkedPos - consumed);
						size_t chunkSize = strtoul(chunkSizeStr.c_str(), NULL, 16);
						if (chunkSize == 0)
						{
							consumed = chunkedPos + 2 + 2;
							this->_requestDataList.back().complete = true;
							break;
						}
						size_t chunkStart = chunkedPos + 2;
						if (this->_readBuffer.size() < chunkStart + chunkSize + 2)
							break;
						tempFile.write(this->_readBuffer.c_str() + chunkStart, chunkSize);
						consumed = chunkStart + chunkSize + 2;
						totalwritedSize += chunkSize;
					}
					this->_requestDataList.back().bodySize += totalwritedSize;
					this->_readBuffer.erase(0, consumed);
					break;
				}
				else
					break;
			}
		}
		else
		{
			ssize_t reqSize = getRequestSize(this->_readBuffer);
			if (reqSize == -1)
				break;
			tReqData reqData;
			reqData.header = this->_readBuffer.substr(0, reqSize);
			reqData.bodyFilePath = "";
			bool isChunked = (reqData.header.find("Transfer-Encoding: chunked") != std::string::npos);
			if (isChunked)
			{
				reqData.bodySize = 0;
				reqData.readedBodySize = 0;
				reqData.complete = false;
				reqData.isChunked = true;
			}
			else if (reqData.header.find("Content-Length: ") != std::string::npos)
			{
				reqData.bodySize = getRequestBodySize(reqData.header);
				reqData.readedBodySize = 0;
				reqData.complete = (reqData.bodySize == 0);
				reqData.isChunked = false;
			}
			else
			{
				reqData.bodySize = 0;
				reqData.readedBodySize = 0;
				reqData.complete = true;
				reqData.isChunked = false;
			}
			this->_readBuffer.erase(0, reqSize);
			this->_requestDataList.push(reqData);
		}
	}
	if (!this->_requestDataList.empty() && this->_requestDataList.back().complete && this->_readBuffer.empty())
		this->_state = HANDLING;
	this->_lastActiveTime = time(NULL);
}

void ClientConnection::handleRead()
{
	if (this->_requestDataList.empty() || !this->_requestDataList.front().complete)
		return;
	RequestParse request(this->_requestDataList.front().header);
	request.setBodyPath(this->_requestDataList.front().bodyFilePath);
	request.setClientMaxBodySize(this->_serverSocket->getConfig().getClientMaxBodySize());
	request.setBodySize(this->_requestDataList.front().bodySize);
	debugLogger("Request header:\n" + this->_requestDataList.front().header);
	ResponseParse response(this->_serverSocket->getConfig());
	response.generateResponse(request);
	this->_responseDataList.push(response);
	this->_state = WRITING_HEADER;
	if (!this->_requestDataList.front().bodyFilePath.empty())
	{
		if (std::remove(this->_requestDataList.front().bodyFilePath.c_str()) != 0)
		{
			debugLogger("Failed to delete temporary file: " + this->_requestDataList.front().bodyFilePath);
		}
	}
	this->_requestDataList.pop();
}


void ClientConnection::addWriteBuffer(const Buffer &buffer)
{
	this->_writeBuffer.append(buffer);
}

void ClientConnection::setWriteBuffer(const Buffer &buffer)
{
	this->_writeBuffer = buffer;
}

void ClientConnection::setState(tConnectionState state)
{
	this->_state = state;
}

const Buffer &ClientConnection::getWriteBuffer()
{
	return this->_writeBuffer;
}

tConnectionState ClientConnection::getState() const
{
	return this->_state;
}

const ServerSocket* ClientConnection::getServerSocket() const
{
	return this->_serverSocket;
}

void ClientConnection::appendRequestData(const tReqData &reqData)
{
	this->_requestDataList.push(reqData);
}

tReqData ClientConnection::getCurrentRequestData() const
{
	if (this->_requestDataList.empty())
		throw std::runtime_error("No request data available");
	return this->_requestDataList.front();
}

void ClientConnection::popCurrentRequestData()
{
	if (this->_requestDataList.empty())
		throw std::runtime_error("No request data to pop");
	this->_requestDataList.pop();
}

bool ClientConnection::requestDataEmpty() const
{
	return this->_requestDataList.empty();
}

ResponseParse &ClientConnection::getCurrentResponseData()
{
	if (this->_responseDataList.empty())
		throw std::runtime_error("No response data available");
	return this->_responseDataList.front();
}

void ClientConnection::appendResponseData(const ResponseParse &responseData)
{
	this->_responseDataList.push(responseData);
}

void ClientConnection::popCurrentResponseData()
{
	if (this->_responseDataList.empty())
		throw std::runtime_error("No response data to pop");
	this->_responseDataList.pop();
}

bool ClientConnection::responseDataEmpty() const
{
	return this->_responseDataList.empty();
}

//* debug
void ClientConnection::setResponseCount(ssize_t count)
{
	this->responseCount = count;
}

ssize_t ClientConnection::getResponseCount() const
{
	return this->responseCount;
}