#include "parser/Config.hpp"
#include <iostream>
#include "core/EventLoop.hpp"
#include <stdexcept>

int main(int argc, char const *argv[])
{
	EventLoop *event = NULL;
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes when writing to closed sockets
	try
	{
		Config _config(argv[1]);
		_config.checkConfigIsValid();
		event = new EventLoop();
		//* add with while and delete with EventLoop destructor
		ServerSocket *socket = new ServerSocket(*_config.getServers().begin());
		socket->open(); // Soketi aç, bind ve listen işlemlerini yapsın
		event->addServerSocket(socket);
		event->run();
	}
	catch (const std::exception &e)
	{
		delete event;
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
