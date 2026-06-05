#include "parser/Route.hpp"

//* Constructor and Destructor

Route::Route() : autoindex(false), redirect(std::make_pair(0, "")), cgi(std::make_pair("", ""))
{
	this->path = "";
	this->root = "";
	this->index = "";
	this->clientMaxBodySize = 0;
	this->methods.clear();
}

Route::Route(const Route &other)
{
	*this = other;
}

Route &Route::operator=(const Route &other)
{
	if (this != &other)
	{
		this->path = other.path;
		this->methods = other.methods;
		this->root = other.root;
		this->index = other.index;
		this->autoindex = other.autoindex;
		this->redirect = other.redirect;
		this->clientMaxBodySize = other.clientMaxBodySize;
		this->cgi = other.cgi;
	}
	return *this;
}

Route::~Route()
{
}

//* Getters and Setters

const std::string &Route::getPath() const
{
	return this->path;
}

const std::set<t_method> &Route::getMethods() const
{
	return this->methods;
}

const std::string &Route::getRoot() const
{
	return this->root;
}

const std::string &Route::getIndex() const
{
	return this->index;
}

bool Route::hasMethod(const t_method method) const
{
	return this->methods.find(method) != this->methods.end();
}

bool Route::isAutoindex() const
{
	return this->autoindex;
}

const std::pair<int, std::string> &Route::getRedirect() const
{
	return this->redirect;
}

const std::pair<std::string, std::string> &Route::getCgi() const
{
	return this->cgi;
}

size_t Route::getClientMaxBodySize() const
{
	return this->clientMaxBodySize;
}

void Route::setPath(const std::string &path)
{
	this->path = path;
}

void Route::insertMethod(const t_method method)
{
	if (method < GET || method > OPTIONS)
		throw LocationAttributeException("Invalid HTTP method");
	bool result = this->methods.insert(method).second;
	if (!result)
		throw LocationAttributeException("Duplicate HTTP method");
}

void Route::setRoot(const std::string &root)
{
	this->root = root;
}

void Route::setIndex(const std::string &index)
{
	this->index = index;
}

void Route::setAutoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void Route::setRedirect(const std::pair<int, std::string> &redirect)
{
	this->redirect = redirect;
}

void Route::setCgi(const std::pair<std::string, std::string> &cgi)
{
	this->cgi = cgi;
}

void Route::setClientMaxBodySize(size_t clientMaxBodySize)
{
	this->clientMaxBodySize = clientMaxBodySize;
}

void Route::checkRouteIsValid() const
{
	if (this->path.empty())
		throw LocationAttributeException("The path is missing");
	if (!this->index.empty() && this->root.empty())
		throw LocationAttributeException("root missing in index configuration");
	if (!this->cgi.first.empty() && this->root.empty())
		throw LocationAttributeException("root missing in CGI configuration");
	if (this->autoindex && this->root.empty())
		throw LocationAttributeException("root missing in autoindex configuration");
	if (this->autoindex && (!this->index.empty() || !this->cgi.first.empty()))
		throw LocationAttributeException("autoindex cannot be enabled with index or CGI configuration");
	if (!this->index.empty() && !this->cgi.first.empty())
		throw LocationAttributeException("index and CGI configuration cannot be enabled at the same time");
	if (this->index.empty() && this->cgi.first.empty() && !this->autoindex && this->redirect.first == 0)
		throw LocationAttributeException("The route must have at least one of the following attributes: index, autoindex, redirect or CGI");
}

//* Exceptions

Route::LocationAttributeException::LocationAttributeException(const std::string &attribute) : attribute(attribute)
{
}

Route::LocationAttributeException::~LocationAttributeException() throw()
{
}

const char *Route::LocationAttributeException::what() const throw()
{
	static std::string message = "Invalid location attribute: ";
	message = message + this->attribute;
	return message.c_str();
}