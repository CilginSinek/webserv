#ifndef _ROUTE_HPP_
#define _ROUTE_HPP_

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <set>

#ifndef T_METHOD
#define T_METHOD
typedef enum Method
{
	GET,
	POST,
	DELETE,
	PUT,
	TRACE,
	HEAD,
	OPTIONS
} t_method;
#endif

/**
 * @brief The Route class represents a route configuration for a web server. It contains information about the path, allowed methods, root directory, index file, autoindex setting, redirect configuration, upload path, and CGI configuration for a specific route.
 * @example
 * like:
 *	location / {
 *		root /var/www/html;
 *		index index.html;
 *		autoindex on;
 *	}
 */
class Route
{
private:
	std::string path;
	std::set<t_method> methods;
	std::string root;
	std::string index;
	bool autoindex;
	std::pair<int, std::string> redirect;
	std::pair<std::string, std::string> cgi;
	size_t clientMaxBodySize;

public:
	Route();
	Route(const Route &other);
	Route &operator=(const Route &other);
	~Route();

	//* Getters and setters for the Route class

	const std::string &getPath() const;
	const std::set<t_method> &getMethods() const;
	bool hasMethod(const t_method method) const;
	const std::string &getRoot() const;
	const std::string &getIndex() const;
	bool isAutoindex() const;
	const std::pair<int, std::string> &getRedirect() const;
	const std::pair<std::string, std::string> &getCgi() const;
	size_t getClientMaxBodySize() const;

	void setPath(const std::string &path);
	void insertMethod(const t_method method);
	void setRoot(const std::string &root);
	void setIndex(const std::string &index);
	void setAutoindex(bool autoindex);
	void setRedirect(const std::pair<int, std::string> &redirect);
	void setCgi(const std::pair<std::string, std::string> &cgi);
	void setClientMaxBodySize(size_t clientMaxBodySize);

	void checkRouteIsValid() const;

	//* Exceptions

	class LocationAttributeException : public std::exception
	{
	private:
		std::string attribute;

	public:
		LocationAttributeException(const std::string &attribute);
		virtual ~LocationAttributeException() throw();
		virtual const char *what() const throw();
	};
};

#endif