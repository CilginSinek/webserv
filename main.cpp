#include "parser/Config.hpp"
#include <iostream>
#include "core/EventLoop.hpp"
#include <stdexcept>
#include <csignal>
#include <vector>

static volatile sig_atomic_t g_shouldStop = 0;

static void handleShutdownSignal(int)
{
	g_shouldStop = 1;
}

static bool sameListen(const ServerSocket &socket, const ServerConfig &server)
{
	return socket.getConfig().getServerIp() == server.getServerIp()
		&& socket.getConfig().getPort() == server.getPort();
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes when writing to closed sockets
	signal(SIGINT, handleShutdownSignal);
	signal(SIGTERM, handleShutdownSignal);
	try
	{
		std::string configPath = argv[1];
		EventLoop event;
		Config config(configPath);
		std::vector<ServerSocket> sockets;
		const std::vector<ServerConfig> &servers = config.getServers();

		event.setStopSignal(&g_shouldStop);
		sockets.reserve(servers.size());
		for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it)
		{
			bool addedToExistingSocket = false;
			for (std::vector<ServerSocket>::iterator socketIt = sockets.begin(); socketIt != sockets.end(); ++socketIt)
			{
				if (sameListen(*socketIt, *it))
				{
					socketIt->addServerConfig(*it);
					addedToExistingSocket = true;
					break;
				}
			}
			if (!addedToExistingSocket)
			{
				sockets.push_back(ServerSocket(*it));
				sockets.back().open();
				event.addServerSocket(&sockets.back());
			}
		}
		event.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
