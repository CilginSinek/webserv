#include "parser/RequestParse.hpp"
#include <sstream>

RequestParse::RequestParse()
{
}

RequestParse::RequestParse(Buffer buffer) : _buffer(buffer)
{
}

RequestParse::RequestParse(const RequestParse &other)
{
	*this = other;
}

RequestParse &RequestParse::operator=(const RequestParse &other)
{
	if (this != &other)
	{
		this->_buffer = other._buffer;
		this->_method = other._method;
		this->_path = other._path;
		this->_version = other._version;
		this->_headers = other._headers;
		this->_query = other._query;
		this->_body = other._body;
	}
	return *this;
}

RequestParse::~RequestParse()
{
}

void RequestParse::setBuffer(Buffer buffer)
{
	this->_buffer = buffer;
}

const Buffer &RequestParse::getBuffer() const
{
	return this->_buffer;
}

const t_method &RequestParse::getMethod() const
{
	return this->_method;
}

const std::string &RequestParse::getPath() const
{
	return this->_path;
}

const std::string &RequestParse::getVersion() const
{
	return this->_version;
}

std::string RequestParse::getQuery() const
{
	return this->_query;
}

const std::map<std::string, std::string> &RequestParse::getHeaders() const
{
	return this->_headers;
}

bool RequestParse::setAndValidFLine(const std::string &firstLine)
{
	std::string method = firstLine.substr(0, firstLine.find(" "));
	if (method == "GET")
		this->_method = GET;
	else if (method == "POST")
		this->_method = POST;
	else if (method == "DELETE")
		this->_method = DELETE;
	else if (method == "PUT")
		this->_method = PUT;
	else if (method == "TRACE")
		this->_method = TRACE;
	else if (method == "HEAD")
		this->_method = HEAD;
	else if (method == "OPTIONS")
		this->_method = OPTIONS;
	else
		return false;
	std::string strWithoutMethod = firstLine.substr(firstLine.find(" ") + 1);
	//* get query
	std::string path = strWithoutMethod.substr(0, strWithoutMethod.find(" "));
	if (path.empty())
		return false;
	if (path[0] != '/' && path[0] != '*')
		return false;
	if (path.find("..") != std::string::npos)
		return false;
	if (path.find("//") != std::string::npos)
		return false;
	if (path.find("?") != std::string::npos)
	{
		this->_query = path.substr(path.find("?") + 1);
		path = path.substr(0, path.find("?"));
	}
	this->_path = path;
	std::string strWithoutPath = strWithoutMethod.substr(strWithoutMethod.find(" ") + 1);
	std::string version = strWithoutPath.substr(0, strWithoutPath.find("\n"));
	if (version != "HTTP/1.1")
		return false;
	this->_version = version;
	return true;
}

void RequestParse::addHeader(std::string key, std::string value)
{
	this->_headers[key] = value;
}

bool RequestParse::setAndValidHeaders(const Buffer &buffer)
{
	std::string headers = buffer.substr(0, buffer.find("\r\n\r\n"));
	std::istringstream stream(headers.c_str());
	std::string line;
	bool firstLine = true;
	while (std::getline(stream, line))
	{
		if (line.empty())
			return false;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line = line.substr(0, line.size() - 1);
		if (firstLine)
		{
			if (!setAndValidFLine(line))
				return false;
			firstLine = false;
			continue;
		}
		size_t colonPos = line.find(":");
		if (colonPos == std::string::npos)
			return false;
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		if (key.empty() || value.empty())
			return false;
		this->addHeader(key, value);
	}
	return true;
}

#include "utils/Utils.hpp"
bool RequestParse::isValid()
{
	std::string bufferStr = this->_buffer.c_str();
	if (this->_buffer.empty())
		return false;
	if (!this->setAndValidHeaders(this->_buffer))
		return false;
	Buffer bodyBuffer = this->_buffer.substr(this->_buffer.find("\r\n\r\n") + 4);
	this->_body = bodyBuffer;
	return true;
}
