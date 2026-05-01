#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include "Server.hpp"

class Config
{
private:
	std::vector<Server> servers;
public:
	Config();
	Config(const std::string& config_file);
	~Config();

};

#endif