#include <map>
#include <vector>
#include "../network/ClientConnection.hpp"
#include "../network/ServerSocket.hpp"
#include "../parser/RequestParse.hpp"
#include "../utils/Utils.hpp"
#include <csignal>

class EventLoop
{
private:
	int epollFd;
	std::map <int, ClientConnection> _connections;
	std::vector <ServerSocket *> _serverSockets;
	volatile sig_atomic_t *_stopSignal;
public:
	EventLoop(/* args */);
	~EventLoop();

	void	setStopSignal(volatile sig_atomic_t *stopSignal);
	void 	addServerSocket(ServerSocket  *socket);
	int		addConnection(int fd, u_int32_t events);
	void	modifyConnection(int fd, u_int32_t events);
	void	removeConnection(int fd);
	void	closeConnection(int fd);
	void	closeIdleConnections();
	void	run();
	void	handleServerSocket(ServerSocket *socket);
	void	handleClientEvent(int fd, u_int32_t events);
};
