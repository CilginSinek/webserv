#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include "ServerConfig.hpp"

class Config
{
private:
	std::vector<ServerConfig> servers;

	void parseConfig(Config &cf, const std::vector<std::string> &tokens);
	void parseServerAttr(Config &cf, const std::vector<std::string> &tokens, size_t &i);
	void parseRoute(Config &cf, const std::vector<std::string> &tokens, size_t &i);
public:
	Config();
	Config(const std::string& config_file);
	~Config();
	void checkConfigIsValid() const;
	const std::vector<ServerConfig> &getServers() const;

};


#endif