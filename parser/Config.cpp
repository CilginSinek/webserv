#include "parser/Config.hpp"
#include "utils/Utils.hpp"
//* Constructor and Destructor

Config::Config()
{
	ServerConfig default_server;
	default_server.setPort(8080);
	default_server.setServerName("localhost");
	default_server.setKeepaliveTimeout(5);
	Route default_route;
	default_route.setPath("/");
	default_route.setRoot("./");
	default_route.setAutoindex(true);
	default_server.insertRoute(default_route);
	this->servers.push_back(default_server);
}

static std::string readConfFile(const std::string &config_file)
{
	//* access check
	if (access(config_file.c_str(), F_OK) != 0)
		throw std::runtime_error("Config file '" + config_file + "' is does not exist");
	if (access(config_file.c_str(), R_OK) != 0)
		throw std::runtime_error("Config file '" + config_file + "' is not readable");
	std::ifstream file(config_file.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open config file: " + config_file);
	std::stringstream buffer;
	std::string line;
	while (getline(file, line))
	{
		if (isIgnoredLine(line))
			continue;
		buffer << line << "\n";
	}
	if (buffer.str().empty())
		throw std::runtime_error("Config file '" + config_file + "' is empty");
	return buffer.str();
}

static std::vector<std::string> splitConfString(const std::string &conf_str)
{
	std::vector<std::string> res;
	std::string current;

	size_t i = 0;
	while (i < conf_str.size())
	{
		char c = conf_str[i];
		if (c == ' ' || (c >= 9 && c <= 13))
		{
			if (!current.empty())
			{
				res.push_back(current);
				current.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (!current.empty())
			{
				res.push_back(current);
				current.clear();
			}
			res.push_back(std::string(1, c));
		}
		else
			current += c;
		i++;
	}
	if (!current.empty())
	{
		res.push_back(current);
	}
	return res;
}

void Config::parseServerAttr(Config &cf, const std::vector<std::string> &tokens, size_t &i)
{
	if (tokens[i] == "listen")
	{
		i++;
		if (i >= tokens.size())
			throw std::runtime_error("Expected port number for listen but got EOF");
		std::string ip_port = tokens[i];
		if (cf.servers.back().getPort() != 0)
			throw std::runtime_error("Duplicate listen directive in server block");
		if (cf.servers.back().getServerIp() != "")
			throw std::runtime_error("Duplicate listen directive in server block");
		size_t colon_pos = ip_port.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string ip = ip_port.substr(0, colon_pos);
			std::string port_str = ip_port.substr(colon_pos + 1);
			int port;
			try
			{
				port = ft_stoi(port_str);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Expected integer port number for listen but got '" + port_str + "'");
			}
			cf.servers.back().setServerIp(ip);
			cf.servers.back().setPort(port);
		}
		else
		{
			int port;
			try
			{
				port = ft_stoi(ip_port);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Expected integer port number for listen but got '" + ip_port + "'");
			}
			cf.servers.back().setServerIp("0.0.0.0");
			cf.servers.back().setPort(port);
		}
		i++;
		i++; // Skip ';'
	}
	else if (tokens[i] == "server_name")
	{
		i++;
		if (i >= tokens.size())
			throw std::runtime_error("Expected server name for server_name but got EOF");
		cf.servers.back().setServerName(tokens[i]);
		i++;
		i++; // Skip ';'
	}
	else if (tokens[i] == "keepalive_timeout")
	{
		i++;
		if (i >= tokens.size())
			throw std::runtime_error("Expected timeout value for keepalive_timeout but got EOF");
		int timeout;
		try
		{
			timeout = ft_stoi(tokens[i]);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("Expected integer timeout value for keepalive_timeout but got '" + tokens[i] + "'");
		}
		cf.servers.back().setKeepaliveTimeout(timeout);
		i++;
		i++; // Skip ';'
	}
	else if (tokens[i] == "error_page")
	{
		i++;
		if (i >= tokens.size())
			throw std::runtime_error("Expected status code for error_page but got EOF");
		size_t j = i;
		while (j < tokens.size() && tokens[j] != ";")
			j++;
		if (j >= tokens.size())
			throw std::runtime_error("Expected ';' after error_page values but got EOF");
		if (j == i)
			throw std::runtime_error("Expected at least one status code for error_page but got none");
		std::vector<int> status_codes;
		for (size_t k = i; k < j - 1; k++)
		{
			try
			{
				int code = ft_stoi(tokens[k]);
				status_codes.push_back(code);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Expected integer status code for error_page but got '" + tokens[k] + "'");
			}
		}
		if (status_codes.empty())
			throw std::runtime_error("Expected at least one status code for error_page but got none");
		std::string page_path = tokens[j - 1];
		for (size_t k = 0; k < status_codes.size(); k++)
		{
			cf.servers.back().insertErrorPage(status_codes[k], page_path);
		}
		i = j + 1; // Skip status codes and page path and ';'
	}
	else if (tokens[i] == "location")
	{
		i++;
		parseRoute(cf, tokens, i);
	}
	else if (tokens[i] == "client_max_body_size")
	{
		i++;
		if (i >= tokens.size())
			throw std::runtime_error("Expected size value for client_max_body_size but got EOF");
		std::string body_size = tokens[i];
		try
		{
			int size = ft_stoi(body_size);
			if (size < 0)
				throw std::runtime_error("client_max_body_size cannot be negative");
			cf.servers.back().setClientMaxBodySize(size);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("Expected integer size value for client_max_body_size but got '" + body_size + "'");
		}
		i++;
		i++; // Skip ';')
	}
	else
	{
		throw std::runtime_error("Unexpected token in server block: '" + tokens[i] + "'");
	}
}

void Config::parseRoute(Config &cf, const std::vector<std::string> &tokens, size_t &i)
{
	Route route;

	route.setClientMaxBodySize(cf.servers.back().getClientMaxBodySize());
	if (i >= tokens.size())
		throw std::runtime_error("Expected path for location but got EOF");
	route.setPath(tokens[i]);
	i++;
	if (i >= tokens.size() || tokens[i] != "{")
		throw std::runtime_error("Expected '{' after location path but got '" + (i < tokens.size() ? tokens[i] : "EOF") + "'");
	i++; // Skip '{'
	while (i < tokens.size() && tokens[i] != "}")
	{
		if (tokens[i] == "root")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected root value but got EOF");
			route.setRoot(tokens[i]);
			i++;
			i++; // Skip ';'
		}
		else if (tokens[i] == "autoindex")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected autoindex value but got EOF");
			std::string val = tokens[i];
			if (val == "on")
				route.setAutoindex(true);
			else if (val == "off")
				route.setAutoindex(false);
			else
				throw std::runtime_error("Expected 'on' or 'off' for autoindex but got '" + val + "'");
			i++;
			i++; // Skip ';'
		}
		else if (tokens[i] == "index")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected index value but got EOF");
			route.setIndex(tokens[i]);
			i++;
			i++; // Skip ';'
		}
		else if (tokens[i] == "methods")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected method value but got EOF");
			while (i < tokens.size() && tokens[i] != ";")
			{
				if (tokens[i] == "GET")
					route.insertMethod(GET);
				else if (tokens[i] == "POST")
					route.insertMethod(POST);
				else if (tokens[i] == "DELETE")
					route.insertMethod(DELETE);
				else if (tokens[i] == "PUT")
					route.insertMethod(PUT);
				else if (tokens[i] == "TRACE")
					route.insertMethod(TRACE);
				else if (tokens[i] == "HEAD")
					route.insertMethod(HEAD);
				else if (tokens[i] == "OPTIONS")
					route.insertMethod(OPTIONS);
				else
					throw std::runtime_error("Unexpected method: '" + tokens[i] + "'");
				i++;
			}
			if (i >= tokens.size() || tokens[i] != ";")
				throw std::runtime_error("Expected ';' after method values but got '" + (i < tokens.size() ? tokens[i] : "EOF") + "'");
			i++; // Skip ';'
		}
		else if (tokens[i] == "return")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected status code for return but got EOF");
			int status_code;
			try
			{
				status_code = ft_stoi(tokens[i]);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Expected integer status code for return but got '" + tokens[i] + "'");
			}
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected URL for return but got EOF");
			std::string url = tokens[i];
			route.setRedirect(std::make_pair(status_code, url));
			i++;
			i++; // Skip ';'
		}
		else if (tokens[i] == "cgi")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected extension for cgi but got EOF");
			std::string extension = tokens[i];
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected script path for cgi but got EOF");
			std::string script_path = tokens[i];
			route.setCgi(std::make_pair(extension, script_path));
			i++;
			i++; // Skip ';'
		}
		else if (tokens[i] == "client_max_body_size")
		{
			i++;
			if (i >= tokens.size())
				throw std::runtime_error("Expected size value for client_max_body_size but got EOF");
			std::string body_size = tokens[i];
			try
			{
				int size = ft_stoi(body_size);
				if (size < 0)
					throw std::runtime_error("client_max_body_size cannot be negative");
				route.setClientMaxBodySize(size);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Expected integer size value for client_max_body_size but got '" + body_size + "'");
			}
			i++;
			i++; // Skip ';')
		}
		else
		{
			throw std::runtime_error("Unexpected token in route block: '" + tokens[i] + "'");
		}
	}
	if (i >= tokens.size() || tokens[i] != "}")
		throw std::runtime_error("Expected '}' at end of location block but got '" + (i < tokens.size() ? tokens[i] : "EOF") + "'");
	i++; // Skip '}'
	cf.servers.back().insertRoute(route);
}

void Config::parseConfig(Config &cf, const std::vector<std::string> &tokens)
{
	size_t i = 0;
	while (i < tokens.size())
	{
		if (tokens[i] != "server")
		{
			throw std::runtime_error("Expected 'server' but got '" + tokens[i] + "'");
		}
		//* Parse server block
		i++; // Skip 'server'
		cf.servers.push_back(ServerConfig());
		if (i >= tokens.size() || tokens[i] != "{")
		{
			throw std::runtime_error("Expected '{' after 'server' but got '" + (i < tokens.size() ? tokens[i] : "EOF") + "'");
		}
		i++; // Skip '{'
		while (i < tokens.size() && tokens[i] != "}")
		{
			parseServerAttr(cf, tokens, i);
		}
		if (i >= tokens.size() || tokens[i] != "}")
			throw std::runtime_error("Expected '}' at end of server block but got '" + (i < tokens.size() ? tokens[i] : "EOF") + "'");
		i++; // Skip '}'
	}
}

Config::Config(const std::string &config_file)
{
	std::vector<std::string> tokens = splitConfString(readConfFile(config_file));
	try
	{
		parseConfig(*this, tokens);
	}
	catch (const std::exception &e)
	{
		throw std::runtime_error("Error parsing config file: " + std::string(e.what()));
	}
}

Config::~Config()
{
}

const std::vector<ServerConfig> &Config::getServers() const
{
	return this->servers;
}

void Config::checkConfigIsValid() const
{
	for (size_t i = 0; i < this->servers.size(); i++)
	{
		this->servers[i].checkIsValidServer();
	}
}

