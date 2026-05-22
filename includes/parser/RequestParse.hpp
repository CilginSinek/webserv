#ifndef _REQUEST_PARSE_HPP_
#define _REQUEST_PARSE_HPP_

#include "Buffer.hpp"
#include "ServerConfig.hpp" 

class RequestParse
{
private:
	Buffer _buffer;
	t_method _method;
	std::string _path;
	std::string _version;
	std::unordered_map<std::string, std::string> _headers;
	Buffer _body;
	ServerConfig &_config;

	bool setAndValidFLine(const std::string &firstLine);
	bool setAndValidHeaders(const Buffer &headersBuffer);

public:
	RequestParse();
	RequestParse(const ServerConfig &config);
	RequestParse(const ServerConfig &config, Buffer buffer);
	~RequestParse();

	void setBuffer(Buffer buffer);
	const Buffer &getBuffer() const;
	const t_method &getMethod() const;
	const std::string &getPath() const;
	const std::string &getVersion() const;
	const std::unordered_map<std::string, std::string> &getHeaders() const;
	const void addHeader(const std::string &key, const std::string &value);
	const bool isValid() const;
};

#endif