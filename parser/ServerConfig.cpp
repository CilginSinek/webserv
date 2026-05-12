#include "ServerConfig.hpp"
#include "Utils.hpp"

//* Constructor and Destructor
ServerConfig::ServerConfig() : keepalive_timeout(0)
{
	this->server_ip = "0.0.0.0";
	this->port = 3131;
	this->server_name = "localhost";
	this->error_pages.clear();
	this->routes.clear();
	this->client_max_body_size = 1048576; // Default to 1MB
}

ServerConfig::~ServerConfig()
{
}

//* Getters and Setters
const std::string &ServerConfig::getServerIp() const
{
	return this->server_ip;
}

int ServerConfig::getPort() const
{
	return this->port;
}

const std::string &ServerConfig::getServerName() const
{
	return this->server_name;
}

const std::map<int, std::string> &ServerConfig::getErrorPages() const
{
	return this->error_pages;
}

int ServerConfig::getKeepaliveTimeout() const
{
	return this->keepalive_timeout;
}

const std::map<std::string, Route> &ServerConfig::getRoutes() const
{
	return this->routes;
}

size_t ServerConfig::getClientMaxBodySize() const
{
	return this->client_max_body_size;
}

void ServerConfig::setServerIp(const std::string &server_ip)
{
	this->server_ip = server_ip;
}

void ServerConfig::setPort(int port)
{
	this->port = port;
}

void ServerConfig::setServerName(const std::string &server_name)
{
	this->server_name = server_name;
}

void ServerConfig::insertErrorPage(int status_code, const std::string &path)
{
	if (status_code < 400 || status_code > 599)
		throw ServerConfigException("Invalid status code for error page: " + ft_itos(status_code));
	bool result = this->error_pages.insert(std::make_pair(status_code, path)).second;
	if (!result)
		throw ServerConfigException("Duplicate status code for error page: " + ft_itos(status_code));
}

void ServerConfig::setKeepaliveTimeout(int keepalive_timeout)
{
	this->keepalive_timeout = keepalive_timeout;
}

void ServerConfig::setClientMaxBodySize(size_t client_max_body_size)
{
	this->client_max_body_size = client_max_body_size;
}

void ServerConfig::insertRoute(const Route &route)
{
	const std::string &path = route.getPath();
	bool result = this->routes.insert(std::make_pair(path, route)).second;
	if (!result)
		throw ServerConfigException("Duplicate route path: " + path);
}

void ServerConfig::checkIsValidServer() const
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

ServerConfig::ServerConfigException::ServerConfigException(const std::string &message) : message(message)
{
}

ServerConfig::ServerConfigException::~ServerConfigException() throw()
{
}

const char *ServerConfig::ServerConfigException::what() const throw()
{
	static std::string Nmessage;

	Nmessage = "Server configuration error: " + this->message;

	return Nmessage.c_str();
}
