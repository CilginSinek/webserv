#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"
//* Constructor and Destructor
Server::Server() : keepalive_timeout(0)
{
	this->server_ip = "0.0.0.0";
	this->port = 3131;
	this->server_name = "localhost";
	this->error_pages.clear();
	this->routes.clear();
	this->client_max_body_size = 1048576; // Default to 1MB
}

Server::~Server()
{
}

//* Getters and Setters

const std::string &Server::getServerIp() const
{
	return this->server_ip;
}

int Server::getPort() const
{
	return this->port;
}

const std::string &Server::getServerName() const
{
	return this->server_name;
}

const std::map<int, std::string> &Server::getErrorPages() const
{
	return this->error_pages;
}

int Server::getKeepaliveTimeout() const
{
	return this->keepalive_timeout;
}

const std::map<std::string, Route> &Server::getRoutes() const
{
	return this->routes;
}

size_t Server::getClientMaxBodySize() const
{
	return this->client_max_body_size;
}

void Server::setServerIp(const std::string &server_ip)
{
	this->server_ip = server_ip;
}

void Server::setPort(int port)
{
	this->port = port;
}

void Server::setServerName(const std::string &server_name)
{
	this->server_name = server_name;
}

void Server::insertErrorPage(int status_code, const std::string &path)
{
	if (status_code < 400 || status_code > 599)
		throw ServerConfigException("Invalid status code for error page: " + ft_itos(status_code));
	bool result = this->error_pages.insert(std::make_pair(status_code, path)).second;
	if (!result)
		throw ServerConfigException("Duplicate status code for error page: " + ft_itos(status_code));
}

void Server::setKeepaliveTimeout(int keepalive_timeout)
{
	this->keepalive_timeout = keepalive_timeout;
}

void Server::setClientMaxBodySize(size_t client_max_body_size)
{
	this->client_max_body_size = client_max_body_size;
}

void Server::insertRoute(const Route &route)
{
	const std::string &path = route.getPath();
	bool result = this->routes.insert(std::make_pair(path, route)).second;
	if (!result)
		throw ServerConfigException("Duplicate route path: " + path);
}

void Server::checkIsValidServer() const
{
	if (this->server_ip.empty())
		throw ServerConfigException("Server IP cannot be empty");
	if (this->port <= 0 || this->port > 65535)
		throw ServerConfigException("Server port is invalid");
	if (this->server_name.empty())
		throw ServerConfigException("Server name cannot be empty");
	if (this->keepalive_timeout < 0)
		throw ServerConfigException("Keepalive timeout cannot be negative");
	if (this->client_max_body_size == 0)
		throw ServerConfigException("Client max body size cannot be zero");
}

//* Exception class for invalid server configuration

Server::ServerConfigException::ServerConfigException(const std::string &message) : message(message)
{
}

Server::ServerConfigException::~ServerConfigException() throw()
{
}

const char *Server::ServerConfigException::what() const throw()
{
	std::string Nmessage = "Server configuration error: " + this->message;
	return Nmessage.c_str();
}