#ifndef _SERVERSOCKET_HPP_
#define _SERVERSOCKET_HPP_

#include "parser/ServerConfig.hpp"
#include "Session.hpp"
#include <vector>

class ServerSocket
{
private:
    int _fd;
    std::map<std::string, Session> _sessions;
    std::vector<ServerConfig> _configs;

public:
    ServerSocket(ServerConfig serverConfig);
    ~ServerSocket();

    void    open();
    void    addServerConfig(const ServerConfig &serverConfig);
    int     acceptClient();
    void    close();
    void    setNonBlocking(int fd);
	const   ServerConfig& getConfig() const;
    Session getAddSession(const std::string &id);
    void    compareAndSetSession(const Session &session);
    void    cleanSessions();
	const   ServerConfig& getConfigForHost(const std::string &host) const;
    int     getFd();
};

#endif
