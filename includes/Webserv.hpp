#include "Config.hpp"
#include "EventLoop.hpp"
#include "ServerSocket.hpp"

#include <vector>

class Webserv
{
private:
    Config                      _config;
    EventLoop                   _eventLoop;
    std::vector<ServerSocket*>  _servers;

    void		createServerSockets();

public:
    Webserv();
    ~Webserv();

    void		init(std::string configPath);
    void		run();
    void		stop();
};
