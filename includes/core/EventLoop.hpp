#include <map>
#include <vector>
#include "AConnection.hpp"
#include "ServerSocket.hpp"

class EventLoop
{
private:
	int epollFd;
	std::map <int, AConnection*> connections;
	std::vector <ServerSocket *> _serverSockets;
public:
	EventLoop(/* args */);
	~EventLoop();

	void 	addServerSocket(ServerSocket  *socket);
	int		addConnection(int fd, u_int32_t events);
	void	modifyConnection(int fd, u_int32_t events);
	void	removeConnection(int fd);
	void	run();
	void	handleServerSocket(ServerSocket *socket);
	void	handleClientEvent(int fd, u_int32_t events);
};
