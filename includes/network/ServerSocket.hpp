#ifndef _SERVERSOCKET_HPP_
#define _SERVERSOCKET_HPP_

#include "parser/ServerConfig.hpp"
#include "Session.hpp"

class ServerSocket
{
private:
    int _fd;
    std::map<std::string, Session> _sessions;
    ServerConfig _config;

public:
    ServerSocket(ServerConfig serverConfig);
    ~ServerSocket();

    void    open();
    int     acceptClient();
    void    close();
    void    setNonBlocking(int fd);
	const   ServerConfig& getConfig() const;
    Session getAddSession(const std::string &id);
    void    compareAndSetSession(const Session &session);
    void    cleanSessions();
    int     getFd();
};

#endif