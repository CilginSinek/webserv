#include "../includes/Config.hpp"

//* Constructor and Destructor

Config::Config()
{
	Server default_server;
	default_server.setPort(80);
	default_server.setServerName("localhost");
	default_server.setKeepaliveTimeout(5);
	Route default_route;
	default_route.setPath("/");
	default_route.setRoot("./");
	default_route.setAutoindex(true);
	default_server.insertRoute(default_route);
	this->servers.push_back(default_server);
}

static std::string readConfFile(const std::string& config_file)
{
	std::ifstream file(config_file);
	if (!file.is_open())
		throw std::runtime_error("Could not open config file: " + config_file);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

static std::vector<std::string> splitConfString(const std::string& conf_str)
{
	std::vector<std::string> res;
	std::string current;

	int i = 0;
	while(i < conf_str.size())
	{
		char c = conf_str[i];
		if (c == ' ' || (c >= 9 && c <= 13))
		{
			if(!current.empty())
			{
				res.push_back(current);
				current.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if(!current.empty())
			{
				res.push_back(current);
				current.clear();
			}
			res.push_back(std::string(1, c));
		}
		else
			current += c;
	}
	if(!current.empty())
	{
		res.push_back(current);
	}
	return res;
}

Config::Config(const std::string& config_file)
{
	std::vector<std::string> tokens = splitConfString(readConfFile(config_file));

}


Config::~Config()
{
}