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

class ResponseParse
{
private:

	const ServerConfig &_serverConfig;
	RequestParse _requestParse;

	Buffer generateDefaultErrorPage(int errorCode) const;
	Buffer cgiExecute(const Route &selectedRoute, std::string requestingPath);
	Buffer autoindexExecute(const Route &selectedRoute, std::string requestingPath);
	Buffer serveFile(const Route &selectedRoute, std::string requestingPath);
	ResponseParse();

public:
    ResponseParse(RequestParse& requestParse, const ServerConfig& config);
	~ResponseParse();

	Buffer generateResponse();
};

#endif