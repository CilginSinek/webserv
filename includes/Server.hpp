#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include "Route.hpp"

class Server
{
private:
	int port;
	std::string server_name;
	//* Error pages are stored in a map where the key is the HTTP status code and the value is the path to the error page location. if the status code is not found in the map, the default error page will be used.
	std::map<int, std::string> error_pages;
	int keepalive_timeout;
	//* Routes are stored in a map where the key is the path and the value is a route object.
	std::map<std::string, Route> routes;
public:
	Server();
	~Server();

	//* Getters and setters for the Server class
	int getPort() const;
	const std::string &getServerName() const;
	const std::map<int, std::string> &getErrorPages() const;
	int getKeepaliveTimeout() const;
	const std::map<std::string, Route> &getRoutes() const;

	void setPort(int port);
	void setServerName(const std::string &server_name);
	void insertErrorPage(int status_code, const std::string &path);
	void setKeepaliveTimeout(int keepalive_timeout);
	void insertRoute(const Route &route);
	void checkIsValidServer() const;

	//* Exception class for invalid server configuration
	class ServerConfigException : public std::exception
	{
	private:
		std::string message;
	public:
		ServerConfigException(const std::string &message);
		virtual const char* what() const throw();
	};

};

#endif