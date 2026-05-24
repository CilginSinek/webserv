#include "parser/ResponseParse.hpp"

ResponseParse::ResponseParse()
{
	throw std::runtime_error("ResponseParse: default constructor is not allowed");
}

ResponseParse::ResponseParse(const RequestParse &requestParse, ServerConfig &serverConfig) : requestParse(requestParse), _serverConfig(serverConfig)
{
}

ResponseParse::~ResponseParse()
{
}

Buffer ResponseParse::generateDefaultErrorPage(int errorCode) const
{
	std::string errorPage = "<html><head><title>" + std::to_string(errorCode) + " Error</title> <link rel=\"icon\" type=\"image/x-icon\" href=\"data:image/x-icon;base64," + std::string(W_FAVICON_BASE64) + "\"> </head><body><h1>" + std::to_string(errorCode) + " Error</h1><p>Sorry, an error occurred while processing your request.</p></body></html>";
	std::string responseHeader = "HTTP/1.1 " + std::to_string(errorCode) + " Error\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(errorPage.size()) + "\r\n\r\n";
	return Buffer(responseHeader + errorPage);
}

Buffer ResponseParse::generateResponse() const
{
	//* check if the request is valid
	if (!requestParse.isValid())
	{
		if (_serverConfig.getErrorPages().find(400) != _serverConfig.getErrorPages().end())
		{
			std::string errorPagePath = _serverConfig.getErrorPages().at(400);
			std::ifstream errorPageFile(errorPagePath);
			if (errorPageFile.is_open())
			{
				std::stringstream buffer;
				buffer << errorPageFile.rdbuf();
				errorPageFile.close();
				size_t contentLength = buffer.str().size();
				std::string responseHeader = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: ";
				responseHeader = responseHeader + std::to_string(contentLength) + "\r\n\r\n";
				return Buffer(responseHeader + buffer.str());
			}
			else
				return generateDefaultErrorPage(400);
		}
		else
			return generateDefaultErrorPage(400);
	}

	//* check Route existence
	if (this->_serverConfig.getRoutes().find(requestParse.getPath()) == this->_serverConfig.getRoutes().end())
	{
		if (_serverConfig.getErrorPages().find(404) != _serverConfig.getErrorPages().end())
		{
			std::string errorPagePath = _serverConfig.getErrorPages().at(404);
			std::ifstream errorPageFile(errorPagePath);
			if (errorPageFile.is_open())
			{
				std::stringstream buffer;
				buffer << errorPageFile.rdbuf();
				errorPageFile.close();
				size_t contentLength = buffer.str().size();
				std::string responseHeader = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: ";
				responseHeader = responseHeader + std::to_string(contentLength) + "\r\n\r\n";
				return Buffer(responseHeader + buffer.str());
			}
			else
				return generateDefaultErrorPage(404);
		}
		else
			return generateDefaultErrorPage(404);
	}

	//* check if the method is allowed for the requested path
	if(this->_serverConfig.getRoutes().at(requestParse.getPath()).getAllowedMethods().find(requestParse.getMethod()) == this->_serverConfig.getRoutes().at(requestParse.getPath()).getAllowedMethods().end())
	{
		if (_serverConfig.getErrorPages().find(405) != _serverConfig.getErrorPages().end())
		{
			std::string errorPagePath = _serverConfig.getErrorPages().at(405);
			std::ifstream errorPageFile(errorPagePath);
			if (errorPageFile.is_open())
			{
				std::stringstream buffer;
				buffer << errorPageFile.rdbuf();
				errorPageFile.close();
				size_t contentLength = buffer.str().size();
				std::string responseHeader = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: ";
				responseHeader = responseHeader + std::to_string(contentLength) + "\r\n\r\n";
				return Buffer(responseHeader + buffer.str());
			}
			else
				return generateDefaultErrorPage(405);
		}
		else
			return generateDefaultErrorPage(405);
	}

	if (this->_serverConfig.getRoutes().at(requestParse.getPath()).getCgiExtension() != "")
	{
		//* handle CGI request
	}
	else
	{
		if (this->_serverConfig.getRoutes().at(requestParse.getPath()).getRedirection() != "")
		{
			//* handle redirection
		}
		else
		{
			if (this->_serverConfig.getRoutes().at(requestParse.getPath()).isAutoindex())
			{
				//* handle autoindex
			}
			else
			{
				//* serve file
			}
		}
	}
}