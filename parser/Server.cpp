#include "../includes/Server.hpp"

//* Constructor and Destructor
Server::Server() : port(0), keepalive_timeout(0)
{
	this->server_name = "";
	this->error_pages.clear();
	this->routes.clear();
}

Server::~Server()
{
}

//* Getters and Setters

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
		throw ServerConfigException("Invalid status code for error page: " + std::to_string(status_code));
	auto result = this->error_pages.insert(std::make_pair(status_code, path));
	if (!result.second)
		throw ServerConfigException("Duplicate status code for error page: " + std::to_string(status_code));
}

void Server::setKeepaliveTimeout(int keepalive_timeout)
{
	this->keepalive_timeout = keepalive_timeout;
}

void Server::insertRoute(const Route &route)
{
	const std::string &path = route.getPath();
	auto result = this->routes.insert(std::make_pair(path, route));
	if (!result.second)
		throw ServerConfigException("Duplicate route path: " + path);
}

void Server::checkIsValidServer() const
{
	if (this->port <= 0 || this->port > 65535)
		throw ServerConfigException("Invalid port number: " + std::to_string(this->port));
	if (this->server_name.empty())
		throw ServerConfigException("Server name cannot be empty");
	if (this->keepalive_timeout < 0)
		throw ServerConfigException("Keepalive timeout cannot be negative");
}

//* Exception class for invalid server configuration

Server::ServerConfigException::ServerConfigException(const std::string &message) : message(message)
{
}

const char *Server::ServerConfigException::what() const throw()
{
	message = "Server configuration error: " + this->message;
	return message.c_str();
}