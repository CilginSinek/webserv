//*TODO handle 413 error when client max body size is exceeded
//*TODO handle envp for cgi execution

#include "parser/ResponseParse.hpp"
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

ResponseParse::ResponseParse(RequestParse &requestParse, const ServerConfig &config)
	: _serverConfig(config), _requestParse(requestParse)
{
}

ResponseParse::~ResponseParse()
{
}

Buffer ResponseParse::generateDefaultErrorPage(int errorCode) const
{
	std::string errorPage =
		"<html><head><title>" + ft_itos(errorCode) +
		" Error</title>"
		"<link rel=\"icon\" type=\"image/svg+xml\" href=\"data:image/svg+xml;base64," +
		std::string(W_FAVICON_BASE64) +
		"\">"
		"</head><body><h1>" +
		ft_itos(errorCode) +
		" Error</h1><p>Sorry, an error occurred while processing your request.</p></body></html>";
	std::string responseHeader = "HTTP/1.1 " + ft_itos(errorCode) + " Error\r\nContent-Type: text/html\r\nContent-Length: " + ft_itos(errorPage.size()) + "\r\n\r\n";
	if (this->_requestParse.getMethod() == HEAD)
		return Buffer(responseHeader);
	return Buffer(responseHeader + errorPage);
}

Buffer ResponseParse::cgiExecute(const Route &selectedRoute, std::string requestingPath)
{
	std::string cgiPath = selectedRoute.getCgi().second;
	if (access(cgiPath.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404);
	if (access(cgiPath.c_str(), X_OK) == -1)
		return generateDefaultErrorPage(403);
	if (requestingPath.empty() || requestingPath[0] != '/')
		requestingPath = "/" + requestingPath;
	if (requestingPath[requestingPath.size() - 1] == '/')
		requestingPath += selectedRoute.getIndex();
	std::string executeFilePath = selectedRoute.getRoot() + requestingPath;
	if (endsWith(executeFilePath, selectedRoute.getCgi().first) == false)
		executeFilePath += selectedRoute.getCgi().first;

	if (access(executeFilePath.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404);

	int pipefd[2];
	std::string cgiOutput;
	std::vector<std::string> envVars;
	for (std::map<std::string, std::string>::const_iterator it = this->_requestParse.getHeaders().begin(); it != this->_requestParse.getHeaders().end(); ++it)
	{
		std::string envVar = "HTTP_" + upperString(it->first) + "=" + it->second;
		envVars.push_back(envVar);
	}

	envVars.push_back("PATH_INFO=" + executeFilePath);
	envVars.push_back("SCRIPT_FILENAME=" + selectedRoute.getCgi().second);
	envVars.push_back("REQUEST_METHOD=" + this->_requestParse.getMethod());
	envVars.push_back("QUERY_STRING=" + this->_requestParse.getQuery());

	char *env[envVars.size() + 1];
	env[envVars.size()] = NULL;
	for (size_t i = 0; i < envVars.size(); i++)
		env[i] = const_cast<char *>(envVars[i].c_str());

	if (pipe(pipefd) == -1)
		return generateDefaultErrorPage(500);
	pid_t pid = fork();
	if (pid == -1)
		return generateDefaultErrorPage(500);
	else if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);

		close(pipefd[1]);

		char *args[] = {const_cast<char *>(cgiPath.c_str()), const_cast<char *>(executeFilePath.c_str()), NULL};
		execve(cgiPath.c_str(), args, env);
		exit(1);
	}
	else
	{
		close(pipefd[1]);
		int bufsize;
		int status;
		waitpid(pid, &status, 0);
		try
		{
			if (this->_requestParse.getHeaders().find("Content-Length") != this->_requestParse.getHeaders().end())
				bufsize = ft_stoi(this->_requestParse.getHeaders().at("Content-Length"));
			else
				bufsize = 4096;
		}
		catch (...)
		{
			return generateDefaultErrorPage(400);
		}
		char buffer[bufsize];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, bufsize)) > 0)
		{
			cgiOutput.append(buffer, bytesRead);
		}
		close(pipefd[0]);
	}

	std::string contentType;
	if (this->_requestParse.getHeaders().find("Accept") != this->_requestParse.getHeaders().end())
	{
		contentType = this->_requestParse.getHeaders().at("Accept").substr(0, this->_requestParse.getHeaders().at("Accept").find(","));
	}
	else
		contentType = "application/octet-stream";
	std::string responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " + ft_itos(cgiOutput.size()) + "\r\n\r\n";
	if (this->_requestParse.getMethod() == HEAD)
		return Buffer(responseHeader);
	return Buffer(responseHeader + cgiOutput);
}

Buffer ResponseParse::autoindexExecute(const Route &selectedRoute, std::string requestingPath)
{
	std::string path = selectedRoute.getRoot() + "/" + requestingPath;
	if (access(path.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404);
	if (access(path.c_str(), R_OK) == -1)
		return generateDefaultErrorPage(403);
	struct stat pathStat;
	if (stat(path.c_str(), &pathStat) == -1)
		return generateDefaultErrorPage(500);
	//* if file serve file
	if (!S_ISDIR(pathStat.st_mode))
		return serveFile(selectedRoute, requestingPath);
	//* if directory generate autoindex page
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return generateDefaultErrorPage(500);
	struct dirent *entry;
	std::string html = "<html><head><title>Index of " + requestingPath + "</title> <link rel=\"icon\" type=\"image/x-icon\" href=\"data:image/x-icon;base64," + std::string(W_FAVICON_BASE64) + "\"> </head><body><h1>Index of " + requestingPath + "</h1><ul>";
	while ((entry = readdir(dir)) != NULL)
	{
		std::string entryName = entry->d_name;
		if (entryName == ".")
			continue;
		std::string entryPath = path + "/" + entryName;
		struct stat entryStat;
		if (stat(entryPath.c_str(), &entryStat) == -1)
			continue;
		if (S_ISDIR(entryStat.st_mode))
			html += "<li><a href=\"" + entryName + "/\">" + entryName + "/</a></li>";
		else
			html += "<li><a href=\"" + entryName + "\">" + entryName + "</a></li>";
	}
	html += "</ul></body></html>";
	std::string responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ft_itos(html.size()) + "\r\n\r\n";
	closedir(dir);
	if (this->_requestParse.getMethod() == HEAD)
		return Buffer(responseHeader);
	return Buffer(responseHeader + html);
}

static std::string getContentType(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";
	std::string extension = filePath.substr(dotPos + 1);
	if (extension == "html" || extension == "htm")
		return "text/html";
	else if (extension == "css")
		return "text/css";
	else if (extension == "js")
		return "application/javascript";
	else if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	else if (extension == "png")
		return "image/png";
	else if (extension == "gif")
		return "image/gif";
	else if (extension == "svg")
		return "image/svg+xml";
	else if (extension == "ico")
		return "image/x-icon";
	else
		return "application/octet-stream";
}

Buffer ResponseParse::serveFile(const Route &selectedRoute, std::string requestingPath)
{
	requestingPath = "/" + requestingPath;
	if (requestingPath.empty() || requestingPath[requestingPath.size() - 1] == '/')
	{

		requestingPath += selectedRoute.getIndex();
	}
	std::string filePath = selectedRoute.getRoot() + requestingPath;
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) == -1)
		return generateDefaultErrorPage(500);
	if (S_ISDIR(fileStat.st_mode))
	{
		if (requestingPath[requestingPath.size() - 1] != '/')
			requestingPath += "/";
		requestingPath += selectedRoute.getIndex();
		filePath = selectedRoute.getRoot() + requestingPath;
	}
	if (access(filePath.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404);
	if (access(filePath.c_str(), R_OK) == -1)
		return generateDefaultErrorPage(403);

	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return generateDefaultErrorPage(500);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	std::string contentType = getContentType(filePath);
	if (this->_requestParse.getHeaders().find("Accept") != this->_requestParse.getHeaders().end())
	{
		std::string acceptHeader = this->_requestParse.getHeaders().at("Accept");
		std::vector<std::string> acceptedTypes;
		std::istringstream acceptStream(acceptHeader);
		std::string type;
		while (std::getline(acceptStream, type, ','))
		{
			type.erase(type.find_last_not_of(" \t\r\n") + 1);
			type.erase(0, type.find_first_not_of(" \t\r\n"));
			acceptedTypes.push_back(type);
		}
		if (std::find(acceptedTypes.begin(), acceptedTypes.end(), contentType) == acceptedTypes.end() && std::find(acceptedTypes.begin(), acceptedTypes.end(), "*/*") == acceptedTypes.end())
			return generateDefaultErrorPage(406);
	}
	std::string responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " + ft_itos(buffer.str().size()) + "\r\n\r\n";
	if (this->_requestParse.getMethod() == HEAD)
		return Buffer(responseHeader);
	return Buffer(responseHeader + buffer.str());
}

Buffer ResponseParse::generateResponse()
{
	//* check if the request is valid
	if (!this->_requestParse.isValid())
	{
		if (_serverConfig.getErrorPages().find(400) != _serverConfig.getErrorPages().end())
		{
			std::string errorPagePath = _serverConfig.getErrorPages().at(400);
			std::ifstream errorPageFile(errorPagePath.c_str());
			if (errorPageFile.is_open())
			{
				std::stringstream buffer;
				buffer << errorPageFile.rdbuf();
				errorPageFile.close();
				size_t contentLength = buffer.str().size();
				std::string responseHeader = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: ";
				responseHeader = responseHeader + ft_itos(contentLength) + "\r\n\r\n";
				return Buffer(responseHeader + buffer.str());
			}
			else
				return generateDefaultErrorPage(400);
		}
		else
			return generateDefaultErrorPage(400);
	}

	Route selectedRoute;
	std::string requestPath = this->_requestParse.getPath();
	std::string requestingPath;
	while (true)
	{
		if (_serverConfig.getRoutes().find(requestPath) != _serverConfig.getRoutes().end())
		{
			selectedRoute = _serverConfig.getRoutes().at(requestPath);
			break;
		}
		if (_serverConfig.getRoutes().find(requestPath + "/") != _serverConfig.getRoutes().end())
		{
			selectedRoute = _serverConfig.getRoutes().at(requestPath + "/");
			break;
		}
		size_t lastSlashPos = requestPath.find_last_of('/');
		if (lastSlashPos == std::string::npos)
		{
			if (_serverConfig.getRoutes().find("/") != _serverConfig.getRoutes().end())
			{
				selectedRoute = _serverConfig.getRoutes().at("/");
				requestingPath = this->_requestParse.getPath().substr(1);
				break;
			}
			else
				return generateDefaultErrorPage(404);
		}
		std::string tail = requestPath.substr(lastSlashPos);
		requestPath = requestPath.substr(0, lastSlashPos);
		requestingPath = tail + requestingPath;
	}

	if (!selectedRoute.getMethods().empty() && !selectedRoute.hasMethod(this->_requestParse.getMethod()))
		return generateDefaultErrorPage(405);

	if (selectedRoute.getRedirect().first != 0)
	{
		std::string responseHeader = "HTTP/1.1 " + ft_itos(selectedRoute.getRedirect().first) + " Moved Permanently\r\nLocation: " + selectedRoute.getRedirect().second + "\r\n\r\n";
		return Buffer(responseHeader);
	}
	else if (!selectedRoute.getCgi().first.empty())
		return cgiExecute(selectedRoute, requestingPath);
	else if (selectedRoute.isAutoindex())
		return autoindexExecute(selectedRoute, requestingPath);
	else
		return serveFile(selectedRoute, requestingPath);
}
