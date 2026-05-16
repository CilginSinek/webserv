#include "../parser/ServerConfig.hpp"

class ServerSocket
{
private:
    int _fd;
    ServerConfig _config;

public:
    ServerSocket(ServerConfig serverConfig);
    ~ServerSocket();

    void    open();
    int     acceptClient();
    void    close();
    void    setNonBlocking(int fd);
	const   ServerConfig& getConfig();
    int     getFd();
};
