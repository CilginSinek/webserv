#ifndef _RESPONSE_PARSE_HPP_
#define _RESPONSE_PARSE_HPP_

#ifndef W_SVG_H
#define W_SVG_H

#define W_FAVICON_BASE64 "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxMDAgMTAwIiB3aWR0aD0iMTAwJSIgaGVpZ2h0PSIxMDAlIj48cGF0aCBkPSJNMTUgMjUgTDM1IDc1IEw1MCA0NSBMNjUgNzUgTDg1IDI1IiBmaWxsPSJub25lIiBzdHJva2U9IiMwMDk2MzkiIHN0cm9rZS13aWR0aD0iMTIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPjwvc3ZnPg=="

#endif

#include "RequestParse.hpp"
#include "ServerConfig.hpp"
#include "utils/Buffer.hpp"
#include "utils/Utils.hpp"
#include "network/Session.hpp"

typedef struct sResData tResData;


class ResponseParse
{
private:
	const ServerConfig &_serverConfig;

	std::string _header;
	std::string _bodyPath;
	bool _isTemp;
	bool _hasBody;
	std::string _bodyContent;
	ssize_t sentSize;
	Session _session;

	void cgiExecute(const Route &selectedRoute, const Route &selectedCgiRoute, const RequestParse &requestParse, std::string requestingPath);
	void autoindexExecute(const Route &selectedRoute, const RequestParse &requestParse, std::string requestingPath);
	void serveFile(const Route &selectedRoute, const RequestParse &requestParse, std::string requestingPath);
	void generateDefaultErrorPage(int errorCode, t_method method);
	void readCgiOutput(struct stat &st);
	void handleUpload(std::string uploadPath, std::string bodyPath, t_method method);
	void handleRemove(std::string filePath, t_method method);
	ResponseParse();
	int checkBodySize(const std::string &filePath, size_t clientMaxBodySize);
public:
	ResponseParse(const ServerConfig &config, Session &session);
	void setSentSize(ssize_t size);
	ssize_t getSentSize() const;
	const std::string &getHeader() const;
	const std::string &getBodyPath() const;
	bool hasBody() const;
	bool isTemp() const;
	const std::string &getBodyContent() const;
	Session &getSession() const;
	~ResponseParse();

	void generateResponse(RequestParse &requestParse);
};

#endif