#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include "Route.hpp"
#include <map>

class Server
{
private:
	std::string server_ip;
	int port;
	std::string server_name;
	//* Error pages are stored in a map where the key is the HTTP status code and the value is the path to the error page location. if the status code is not found in the map, the default error page will be used.
	std::map<int, std::string> error_pages;
	int keepalive_timeout;
	size_t client_max_body_size;
	//* Routes are stored in a map where the key is the path and the value is a route object.
	std::map<std::string, Route> routes;
public:
	Server();
	~Server();

	//* Getters and setters for the Server class
	const std::string &getServerIp() const;
	int getPort() const;
	const std::string &getServerName() const;
	const std::map<int, std::string> &getErrorPages() const;
	int getKeepaliveTimeout() const;
	size_t getClientMaxBodySize() const;
	const std::map<std::string, Route> &getRoutes() const;

	void setServerIp(const std::string &server_ip);
	void setPort(int port);
	void setServerName(const std::string &server_name);
	void insertErrorPage(int status_code, const std::string &path);
	void setKeepaliveTimeout(int keepalive_timeout);
	void insertRoute(const Route &route);
	void checkIsValidServer() const;
	void setClientMaxBodySize(size_t client_max_body_size);

	//* Exception class for invalid server configuration
	class ServerConfigException : public std::exception
	{
	private:
		std::string message;
	public:
		ServerConfigException(const std::string &message);
		virtual ~ServerConfigException() throw();
		virtual const char* what() const throw();
	};

};

#endif