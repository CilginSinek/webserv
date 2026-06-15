#include "parser/ResponseParse.hpp"
#include "network/ClientConnection.hpp"
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

ResponseParse::ResponseParse(const ServerConfig &config, Session &session)
	: _serverConfig(config), _session(session)
{
	this->_header.clear();
	this->_bodyPath.clear();
	this->_isTemp = false;
	this->_hasBody = false;
	this->_bodyContent.clear();
	this->sentSize = 0;
}

ResponseParse::~ResponseParse()
{
}

void ResponseParse::handleUpload(std::string uploadPath, std::string bodyPath, t_method method)
{
	std::ofstream uploadFile(uploadPath.c_str(), std::ios::binary);
	if (!uploadFile.is_open())
		return generateDefaultErrorPage(500, method);
	std::ifstream bodyFile(bodyPath.c_str(), std::ios::binary);
	if (!bodyFile.is_open())
	{
		uploadFile.close();
		return generateDefaultErrorPage(500, method);
	}
	uploadFile << bodyFile.rdbuf();
	uploadFile.close();
	bodyFile.close();
	this->_session.updateLastAccessTime();
	if (method == POST)
		this->_header = "HTTP/1.1 201 Created\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Length: 0\r\n\r\n";
	else
		this->_header = "HTTP/1.1 200 OK\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Length: 0\r\n\r\n";
}

void ResponseParse::handleRemove(std::string filePath, t_method method)
{
	if (access(filePath.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404, method);
	if (access(filePath.c_str(), W_OK) == -1)
		return generateDefaultErrorPage(403, method);
	if (remove(filePath.c_str()) != 0)
		return generateDefaultErrorPage(500, method);
	this->_session.updateLastAccessTime();
	this->_header = "HTTP/1.1 200 OK\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Length: 0\r\n\r\n";
}

void ResponseParse::generateDefaultErrorPage(int errorCode, t_method method)
{
	struct stat st;
	this->_session.setData("last_error", ft_itos(errorCode));
	this->_session.updateLastAccessTime();
	if (_serverConfig.getErrorPages().find(errorCode) != _serverConfig.getErrorPages().end())
	{
		std::string errorPagePath = _serverConfig.getErrorPages().at(errorCode);
		if (stat(errorPagePath.c_str(), &st) == -1 || S_ISDIR(st.st_mode))
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
			std::string responseHeader = "HTTP/1.1 " + ft_itos(errorCode) + " Error\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Type: text/html\r\nContent-Length: " + ft_itos(errorPage.size()) + "\r\n\r\n";
			this->_header = responseHeader;
			if (method == HEAD)
			{
				this->_hasBody = false;
				return;
			}
			this->_bodyPath = "";
			this->_hasBody = true;
			this->_bodyContent = errorPage;
			return;
		}
		std::string responseHeader = "HTTP/1.1 " + ft_itos(errorCode) + " Error\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Type: text/html\r\nContent-Length: " + ft_itos(st.st_size) + "\r\n\r\n";
		this->_header = responseHeader;
		if (method == HEAD)
		{
			this->_hasBody = false;
			return;
		}
		this->_bodyPath = errorPagePath;
		this->_hasBody = true;
		return;
	}
	std::string errorPage =
		"<html><head><title>" + ft_itos(errorCode) +
		" Error</title>"
		"<link rel=\"icon\" type=\"image/svg+xml\" href=\"data:image/svg+xml;base64," +
		std::string(W_FAVICON_BASE64) +
		"\">"
		"</head><body><h1>" +
		ft_itos(errorCode) +
		" Error</h1><p>Sorry, an error occurred while processing your request.</p></body></html>";
	std::string responseHeader = "HTTP/1.1 " + ft_itos(errorCode) + " Error\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Type: text/html\r\nContent-Length: " + ft_itos(errorPage.size()) + "\r\n\r\n";
	this->_header = responseHeader;
	if (method == HEAD)
	{
		this->_hasBody = false;
		return;
	}
	this->_bodyPath = "";
	this->_hasBody = true;
	this->_bodyContent = errorPage;
	return;
}

void ResponseParse::readCgiOutput(struct stat &st)
{
	char buffer[4096];
	ssize_t bytesRead;
	std::ifstream bodyFile(this->_bodyPath.c_str(), std::ios::binary);
	if (!bodyFile.is_open())
		return;
	std::string responseHeader;
	while (bodyFile)
	{
		bodyFile.read(buffer, sizeof(buffer));
		bytesRead = bodyFile.gcount();
		if (bytesRead <= 0)
			break;
		responseHeader.append(buffer, bytesRead);
		size_t headerEndPos = responseHeader.find("\r\n\r\n");
		if (headerEndPos != std::string::npos)
		{
			this->_header = responseHeader.substr(0, headerEndPos + 4);
			this->sentSize = headerEndPos + 4;
			break;
		}
	}
	bodyFile.close();
	if (this->_header.empty() && !responseHeader.empty())
	{
		this->_header = responseHeader;
		this->sentSize = responseHeader.size();
	}

	if (this->_header.empty())
	{
		this->_header = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
		this->sentSize = 0;
	}
	if (this->_header.compare(0, 5, "HTTP/") != 0)
	{
		std::string appendHeader = "HTTP/1.1";

		size_t statusPos = this->_header.find("Status: ");
		if (statusPos != std::string::npos)
		{
			statusPos += 8;
			size_t statusEndPos = this->_header.find("\r\n", statusPos);
			if (statusEndPos != std::string::npos)
				appendHeader += " " + this->_header.substr(statusPos, statusEndPos - statusPos);
			else
				appendHeader += " 200 OK";
		}
		else
			appendHeader += " 200 OK";
		this->_header = appendHeader + "\r\n" + this->_header;
	}
	if (this->_header.find("Content-Length: ") == std::string::npos)
	{
		size_t headerEndPos = this->_header.find("\r\n\r\n");
		std::string headerContentL =
			"\r\nContent-Length: " + ft_itos(st.st_size - this->sentSize) + "\r\n\r\n";
		if (headerEndPos != std::string::npos)
			this->_header = this->_header.substr(0, headerEndPos) + headerContentL + this->_header.substr(headerEndPos + 4);
		else
			this->_header += headerContentL;
	}
	if (this->_header.find("Set-Cookie: ") == std::string::npos)
	{
		size_t headerEndPos = this->_header.find("\r\n\r\n");
		std::string headerCookie = "Set-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\n";
		if (headerEndPos != std::string::npos)
			this->_header = this->_header.substr(0, headerEndPos) + "\r\n" + headerCookie + "\r\n";
		else
			this->_header += "\r\n" + headerCookie + "\r\n\r\n";
	}
	else
	{
		size_t cookiePos = this->_header.find("Set-Cookie: ");
		size_t cookieEndPos = this->_header.find("\r\n", cookiePos);
		std::string newCookie = "Set-Cookie: WebservSessionId=" + this->_session.getId() + ";";
		if (cookieEndPos != std::string::npos)
			this->_header = this->_header.substr(0, cookiePos) + newCookie + this->_header.substr(cookieEndPos);
		else
			this->_header = this->_header.substr(0, cookiePos) + newCookie + "\r\n";
	}
	std::string tmpHeader = this->_header;
	size_t pos = 0;
	while ((pos = tmpHeader.find("X-CGI-", pos)) != std::string::npos)
	{
		size_t equalPos = tmpHeader.find("=", pos);
		size_t lineEnd = tmpHeader.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			lineEnd = tmpHeader.length();
		if (equalPos == std::string::npos || equalPos > lineEnd)
		{
			pos++;
			continue;
		}
		std::string key = tmpHeader.substr(pos + 6, equalPos - (pos + 6));
		std::string value = tmpHeader.substr(equalPos + 1, lineEnd - (equalPos + 1));
		this->_session.setData(key, value);
		pos = lineEnd;
	}
}

void ResponseParse::cgiExecute(const Route &selectedRoute, const Route &selectedCgiRoute, const RequestParse &requestParse, std::string requestingPath)
{
	std::string cgiPath = selectedCgiRoute.getCgi().second;
	if (access(cgiPath.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404, requestParse.getMethod());
	if (access(cgiPath.c_str(), X_OK) == -1)
		return generateDefaultErrorPage(403, requestParse.getMethod());

	if (requestingPath.empty() || requestingPath[0] != '/')
		requestingPath = "/" + requestingPath;
	if (requestingPath[requestingPath.size() - 1] == '/')
		requestingPath += selectedRoute.getIndex();
	std::string executeFilePath = selectedRoute.getRoot() + requestingPath;
	if (endsWith(executeFilePath, selectedCgiRoute.getCgi().first) == false)
	{
		if (selectedRoute.isUpload())
		{
			if (requestParse.getMethod() == POST || requestParse.getMethod() == PUT)
				return handleUpload(selectedRoute.getRoot() + requestingPath, requestParse.getBodyPath(), requestParse.getMethod());
			else if (requestParse.getMethod() == DELETE)
				return handleRemove(selectedRoute.getRoot() + requestingPath, requestParse.getMethod());
		}
		else
			return serveFile(selectedRoute, requestParse, requestingPath);
	}
	std::vector<std::string> envVars;
	for (std::map<std::string, std::string>::const_iterator it = requestParse.getHeaders().begin(); it != requestParse.getHeaders().end(); ++it)
	{
		std::string envVar = "HTTP_" + normalizeEnv(it->first) + "=" + trim(it->second);
		envVars.push_back(envVar);
	}
	if (requestParse.getMethod() == POST || requestParse.getMethod() == PUT)
	{
		envVars.push_back("CONTENT_LENGTH=" + ft_itos(requestParse.getBodySize()));
		if (requestParse.getHeaders().find("Content-Type") != requestParse.getHeaders().end())
			envVars.push_back("CONTENT_TYPE=" + requestParse.getHeaders().at("Content-Type"));
	}
	envVars.push_back("PATH_INFO=" + trim(requestParse.getPath()));
	envVars.push_back("SCRIPT_NAME=" + trim(requestParse.getPath()));
	envVars.push_back("SERVER_NAME=" + this->_serverConfig.getServerName());
	envVars.push_back("SERVER_PORT=" + ft_itos(this->_serverConfig.getPort()));
	envVars.push_back("SERVER_PROTOCOL=" + requestParse.getVersion());
	envVars.push_back("SERVER_SOFTWARE=webserv/1.0");
	envVars.push_back("REMOTE_ADDR=127.0.0.1");
	envVars.push_back("REQUEST_URI=" + trim(requestParse.getPath()));
	envVars.push_back("REDIRECT_STATUS=200");
	const std::map<std::string, std::string> sessionData = this->_session.getAllData();
	for (std::map<std::string, std::string>::const_iterator it = sessionData.begin(); it != sessionData.end(); ++it)
	{
		std::string envVar = normalizeEnv(it->first) + "=" + trim(it->second);
		envVars.push_back(envVar);
	}
	std::string methodStr;
	switch (requestParse.getMethod())
	{
	case GET:
		methodStr = "GET";
		break;
	case POST:
		methodStr = "POST";
		break;
	case DELETE:
		methodStr = "DELETE";
		break;
	case PUT:
		methodStr = "PUT";
		break;
	case HEAD:
		methodStr = "HEAD";
		break;
	case OPTIONS:
		methodStr = "OPTIONS";
		break;
	default:
		methodStr = "GET";
	}
	envVars.push_back("REQUEST_METHOD=" + methodStr);
	envVars.push_back("QUERY_STRING=" + requestParse.getQuery());

	std::vector<char *> env(envVars.size() + 1, NULL);
	for (size_t i = 0; i < envVars.size(); i++)
		env[i] = const_cast<char *>(envVars[i].c_str());

	//* create temp file for cgi output
	char tempTemplate[] = "./temp/cgi_output_XXXXXX";
	int tempFd = mkstemp(tempTemplate);
	if (tempFd == -1)
		return generateDefaultErrorPage(500, requestParse.getMethod());
	std::string tempFilePath = tempTemplate;
	close(tempFd);
	this->_isTemp = true;
	this->_bodyPath = tempFilePath;
	pid_t pid = fork();
	if (pid == -1)
	{
		return generateDefaultErrorPage(500, requestParse.getMethod());
	}
	else if (pid == 0)
	{
		int inputFd = open(requestParse.getBodyPath().c_str(), O_RDONLY);
		if (inputFd != -1)
		{
			dup2(inputFd, STDIN_FILENO);
		}
		int outputFd = open(tempFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
		if (outputFd == -1)
		{
			close(inputFd);
			debugLogger("Failed to open CGI output file: " + tempFilePath);
			exit(1);
		}
		dup2(outputFd, STDOUT_FILENO);
		dup2(outputFd, STDERR_FILENO);

		char *args[] = {const_cast<char *>(cgiPath.c_str()), const_cast<char *>(executeFilePath.c_str()), NULL};
		execve(cgiPath.c_str(), args, &env[0]);
		exit(1);
	}
	int status;
	waitpid(pid, &status, 0);
	struct stat st;
	if (stat(tempFilePath.c_str(), &st) == -1)
		return generateDefaultErrorPage(500, requestParse.getMethod());
	this->_session.updateLastAccessTime();
	readCgiOutput(st);
	if (requestParse.getMethod() == HEAD)
	{
		std::remove(tempFilePath.c_str());
		this->_isTemp = false;
		return;
	}
	this->_hasBody = true;
}

void ResponseParse::autoindexExecute(const Route &selectedRoute, const RequestParse &requestParse, std::string requestingPath)
{
	std::string path = selectedRoute.getRoot() + requestingPath;
	if (!selectedRoute.getIndex().empty())
	{
		if (!requestingPath.empty())
			return serveFile(selectedRoute, requestParse, requestingPath);
		else if (requestingPath.empty() && access((path + "/" + selectedRoute.getIndex()).c_str(), F_OK) == 0)
			return serveFile(selectedRoute, requestParse, requestingPath);
	}

	if (access(path.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404, requestParse.getMethod());
	if (access(path.c_str(), R_OK) == -1)
		return generateDefaultErrorPage(403, requestParse.getMethod());
	struct stat pathStat;
	if (stat(path.c_str(), &pathStat) == -1)
		return generateDefaultErrorPage(500, requestParse.getMethod());
	//* if file serve file
	if (!S_ISDIR(pathStat.st_mode))
		return serveFile(selectedRoute, requestParse, requestingPath);
	//* if directory generate autoindex page
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return generateDefaultErrorPage(500, requestParse.getMethod());
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
	std::string responseHeader = "HTTP/1.1 200 OK\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Type: text/html\r\nContent-Length: " + ft_itos(html.size()) + "\r\n\r\n";
	this->_session.updateLastAccessTime();
	closedir(dir);
	if (requestParse.getMethod() == HEAD)
	{
		this->_hasBody = false;
		this->_header = responseHeader;
		return;
	}
	this->_hasBody = true;
	this->_header = responseHeader;
	this->_bodyContent = html;
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
	else if (extension == "txt")
		return "text/plain";
	else if (extension == "pdf")
		return "application/pdf";
	else
		return "application/octet-stream";
}

void ResponseParse::serveFile(const Route &selectedRoute, const RequestParse &requestParse, std::string requestingPath)
{
	this->_session.updateLastAccessTime();
	if (requestParse.getMethod() == POST || requestParse.getMethod() == PUT)
	{
		if (selectedRoute.isUpload())
			handleUpload(selectedRoute.getRoot() + requestingPath, requestParse.getBodyPath(), requestParse.getMethod());
		else
			this->_header = "HTTP/1.1 200 OK\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Length: 0\r\n\r\n";
		return;
	}
	if (requestParse.getMethod() == DELETE)
	{
		if (selectedRoute.isUpload())
			handleRemove(selectedRoute.getRoot() + requestingPath, requestParse.getMethod());
		else
			this->_header = "HTTP/1.1 403 Forbidden\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Length: 0\r\n\r\n";
		return;
	}
	if (requestingPath.empty() || requestingPath[0] != '/')
		requestingPath = "/" + requestingPath;
	if (requestingPath[requestingPath.size() - 1] == '/')
		requestingPath += selectedRoute.getIndex();
	std::string filePath = selectedRoute.getRoot() + requestingPath;
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) == -1)
		return generateDefaultErrorPage(404, requestParse.getMethod());
	if (S_ISDIR(fileStat.st_mode))
	{
		if (requestingPath[requestingPath.size() - 1] != '/')
			requestingPath += "/";
		requestingPath += selectedRoute.getIndex();
		filePath = selectedRoute.getRoot() + requestingPath;
		if (stat(filePath.c_str(), &fileStat) == -1 || S_ISDIR(fileStat.st_mode))
			return generateDefaultErrorPage(404, requestParse.getMethod());
	}
	if (access(filePath.c_str(), F_OK) == -1)
		return generateDefaultErrorPage(404, requestParse.getMethod());
	if (access(filePath.c_str(), R_OK) == -1)
		return generateDefaultErrorPage(403, requestParse.getMethod());

	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return generateDefaultErrorPage(500, requestParse.getMethod());
	std::string contentType = getContentType(filePath);
	if (requestParse.getHeaders().find("Accept") != requestParse.getHeaders().end())
	{
		std::string acceptHeader = requestParse.getHeaders().at("Accept");
		std::vector<std::string> acceptedTypes;
		std::istringstream acceptStream(acceptHeader);
		std::string type;
		while (std::getline(acceptStream, type, ','))
		{
			type.erase(type.find_last_not_of(" \t\r\n") + 1);
			type.erase(0, type.find_first_not_of(" \t\r\n"));
			size_t semicolonPos = type.find(';');
			if (semicolonPos != std::string::npos)
				type = type.substr(0, semicolonPos);
			acceptedTypes.push_back(type);
		}
		if (std::find(acceptedTypes.begin(), acceptedTypes.end(), contentType) == acceptedTypes.end() && std::find(acceptedTypes.begin(), acceptedTypes.end(), "*/*") == acceptedTypes.end())
			return generateDefaultErrorPage(406, requestParse.getMethod());
	}
	std::string responseHeader = "HTTP/1.1 200 OK\r\nSet-Cookie: WebservSessionId=" + this->_session.getId() + ";\r\nContent-Type: " + contentType + "\r\nContent-Length: " + ft_itos(fileStat.st_size) + "\r\n\r\n";
	if (requestParse.getMethod() == HEAD)
	{
		this->_hasBody = false;
		this->_header = responseHeader;
		return;
	}
	this->_hasBody = true;
	this->_header = responseHeader;
	this->_bodyPath = filePath;
}

int ResponseParse::checkBodySize(const std::string &filePath, size_t clientMaxBodySize)
{
	struct stat st;
	if (stat(filePath.c_str(), &st) == -1)
		return 500;
	if (clientMaxBodySize != 0 && static_cast<size_t>(st.st_size) > clientMaxBodySize)
		return 413;
	return 200;
}

void ResponseParse::generateResponse(RequestParse &requestParse)
{
	//* check if the request is valid
	int statusCode = requestParse.isValid();
	if (statusCode < 200 || statusCode >= 300)
		return generateDefaultErrorPage(statusCode, requestParse.getMethod());
	Route selectedRoute;
	std::string requestPath = requestParse.getPath();
	std::string requestingPath;

	Route selectedCgiRoute;
	size_t dotPos = requestPath.find_last_of('.');
	size_t lastSlash = requestPath.find_last_of('/');
	if (dotPos != std::string::npos && (lastSlash == std::string::npos || dotPos > lastSlash))
	{
		std::string ext = requestPath.substr(dotPos);
		if (_serverConfig.getRoutes().find(ext) != _serverConfig.getRoutes().end())
		{
			selectedCgiRoute = _serverConfig.getRoutes().at(ext);
		}
	}
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
				requestingPath = requestParse.getPath().substr(1);
				break;
			}
			else
				return generateDefaultErrorPage(404, requestParse.getMethod());
		}
		std::string tail = requestPath.substr(lastSlashPos);
		requestPath = requestPath.substr(0, lastSlashPos);
		requestingPath = tail + requestingPath;
	}

	if (!selectedCgiRoute.getCgi().first.empty() && (!selectedCgiRoute.getMethods().empty() && !selectedCgiRoute.hasMethod(requestParse.getMethod())))
	{
		if ((!selectedRoute.getMethods().empty() && !selectedRoute.hasMethod(requestParse.getMethod())))
			return generateDefaultErrorPage(405, requestParse.getMethod());
	}
	if (selectedCgiRoute.getCgi().first.empty() && selectedRoute.isAutoindex() == false)
	{
		if (!selectedRoute.getMethods().empty() && !selectedRoute.hasMethod(requestParse.getMethod()))
			return generateDefaultErrorPage(405, requestParse.getMethod());
	}
	if (selectedRoute.getRedirect().first != 0)
	{
		std::string responseHeader = "HTTP/1.1 " + ft_itos(selectedRoute.getRedirect().first) + " Moved Permanently\r\nLocation: " + selectedRoute.getRedirect().second + "\r\n\r\n";
		if (!requestParse.getBodyPath().empty())
			std::remove(requestParse.getBodyPath().c_str());
		this->_hasBody = false;
		this->_header = responseHeader;
		return;
	}
	if (!requestParse.getBodyPath().empty())
	{
		int bodySizeCheck = checkBodySize(requestParse.getBodyPath(), selectedRoute.getClientMaxBodySize());
		if (bodySizeCheck != 200)
		{
			std::remove(requestParse.getBodyPath().c_str());
			return generateDefaultErrorPage(bodySizeCheck, requestParse.getMethod());
		}
	}
	if (!selectedCgiRoute.getCgi().first.empty() && (selectedCgiRoute.getMethods().empty() || selectedCgiRoute.hasMethod(requestParse.getMethod())))
		return cgiExecute(selectedRoute, selectedCgiRoute, requestParse, requestingPath);
	else if (selectedRoute.isAutoindex())
		return autoindexExecute(selectedRoute, requestParse, requestingPath);
	else
		return serveFile(selectedRoute, requestParse, requestingPath);
}

const std::string &ResponseParse::getHeader() const
{
	return this->_header;
}

const std::string &ResponseParse::getBodyPath() const
{
	return this->_bodyPath;
}

bool ResponseParse::hasBody() const
{
	return this->_hasBody;
}

bool ResponseParse::isTemp() const
{
	return this->_isTemp;
}

const std::string &ResponseParse::getBodyContent() const
{
	return this->_bodyContent;
}

void ResponseParse::setSentSize(ssize_t size)
{
	this->sentSize = size;
}

ssize_t ResponseParse::getSentSize() const
{
	return this->sentSize;
}


const Session &ResponseParse::getSession() const
{
	return this->_session;
}