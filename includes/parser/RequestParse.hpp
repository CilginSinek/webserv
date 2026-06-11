#ifndef _REQUEST_PARSE_HPP_
#define _REQUEST_PARSE_HPP_

#include "utils/Buffer.hpp"
#include "utils/Utils.hpp"
#include "ServerConfig.hpp"
#include <algorithm>

class RequestParse
{
private:
	Buffer _buffer;
	t_method _method;
	std::string _path;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _query;
	std::string _bodyPath;
	size_t bodySize;
	ssize_t clientMaxBodySize;

	bool setAndValidFLine(const std::string &firstLine);
	bool setAndValidHeaders(const Buffer &headersBuffer);

public:
	RequestParse();
	RequestParse(Buffer buffer);
	RequestParse(const RequestParse &other);
	RequestParse &operator=(const RequestParse &other);
	~RequestParse();

	void setClientMaxBodySize(ssize_t size);
	void setBuffer(Buffer buffer);
	void setBodyPath(const std::string &path);
	void setBodySize(size_t size);
	size_t getBodySize() const;
	const Buffer &getBuffer() const;
	const t_method &getMethod() const;
	const std::string &getPath() const;
	const std::string &getVersion() const;
	std::string getQuery() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBodyPath() const;
	void addHeader(std::string key, std::string value);
	int isValid();
};

#endif