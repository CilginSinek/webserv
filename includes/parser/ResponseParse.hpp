#ifndef _RESPONSE_PARSE_HPP_
#define _RESPONSE_PARSE_HPP_

#ifndef W_SVG_H
#define W_SVG_H

#define W_FAVICON_BASE64 "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxMDAgMTAwIiB3aWR0aD0iMTAwJSIgaGVpZ2h0PSIxMDAlIj48cGF0aCBkPSJNMTUgMjUgTDM1IDc1IEw1MCA0NSBMNjUgNzUgTDg1IDI1IiBmaWxsPSJub25lIiBzdHJva2U9IiMwMDk2MzkiIHN0cm9rZS13aWR0aD0iMTIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPjwvc3ZnPg=="

#endif

class ResponseParse
{
private:
	ServerConfig &_serverConfig;
	RequestParse &requestParse;

	Buffer generateDefaultErrorPage(int errorCode) const;


public:
	ResponseParse();
	ResponseParse(const RequestParse &requestParse, ServerConfig &config);
	~ResponseParse();

	Buffer generateResponse() const;
};

#endif