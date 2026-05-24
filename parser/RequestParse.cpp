#include "parser/RequestParse.hpp"

RequestParse::RequestParse()
{
}

RequestParse::RequestParse(Buffer buffer) : _buffer(buffer)
{
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
	std::string path = strWithoutMethod.substr(0, strWithoutMethod.find(" "));
	if (path.empty())
		return false;
	if (path[0] != '/' && path[0] != '*')
		return false;
	if (path.find("..") != std::string::npos)
		return false;
	if (path.find("//") != std::string::npos)
		return false;
	this->_path = path;
	std::string strWithoutPath = strWithoutMethod.substr(strWithoutMethod.find(" ") + 1);
	std::string version = strWithoutPath.substr(0, strWithoutPath.find("\n"));
	if (version != "HTTP/1.1")
		return false;
	this->_version = version;
	return true;
}

bool RequestParse::setAndValidHeaders(const Buffer &buffer)
{
	std::string headers = buffer.substr(0, buffer.find("\r\n\r\n"));
	std::istringstream stream(headers);
	std::string line;
	bool firstLine = true;
	while (std::getline(stream, line))
	{
		if (line.empty())
			return false;
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
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

const bool RequestParse::isValid() const
{
	if (this->_buffer.empty())
		return false;
	if (!setAndValidHeaders(this->_buffer))
		return false;
	Buffer bodyBuffer = this->_buffer.substr(this->_buffer.find("\r\n\r\n") + 4);
	this->_body = bodyBuffer;
	return true;
}
