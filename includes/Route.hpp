#ifndef _ROUTE_HPP_
#define _ROUTE_HPP_

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
	std::vector<std::string> methods;
	std::string root;
	std::string index;
	bool autoindex;
	std::pair<int, std::string> redirect;
	std::string upload_path;
	std::pair<std::string, std::string> cgi;
public:
	Route();
	~Route();

	// Getters and setters for the Route class
	const std::string& getPath() const;
	const std::vector<std::string>& getMethods() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	bool isAutoindex() const;
	const std::pair<int, std::string>& getRedirect() const;
	const std::string& getUploadPath() const;
	const std::pair<std::string, std::string>& getCgi() const;

	void setPath(const std::string& path);
	void setMethods(const std::vector<std::string>& methods);
	void setRoot(const std::string& root);
	void setIndex(const std::string& index);
	void setAutoindex(bool autoindex);
	void setRedirect(const std::pair<int, std::string>& redirect);
	void setUploadPath(const std::string& upload_path);
	void setCgi(const std::pair<std::string, std::string>& cgi);
};

#endif