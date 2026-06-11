#include "parser/Config.hpp"
#include <iostream>
#include "core/EventLoop.hpp"
#include <stdexcept>
#include <csignal>

static volatile sig_atomic_t g_shouldStop = 0;

static void handleShutdownSignal(int)
{
	g_shouldStop = 1;
}

int main(int argc, char const *argv[])
{
	EventLoop *event = NULL;
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes when writing to closed sockets
	signal(SIGINT, handleShutdownSignal);
	signal(SIGTERM, handleShutdownSignal);
	// if(argc == 2)
	// {
	// 	try
	// 	{
	// 		Config config(argv[1]);
	// 		std::cout << "Config file '" << argv[1] << "' parsed successfully." << std::endl;
	// 		configPrinter(config);
	// 		std::cout << "Config file '" << argv[1] << "' printed successfully." << std::endl;
	// 		config.checkConfigIsValid();
	// 		std::cout << "Config file '" << argv[1] << "' is valid." << std::endl;
	// 	}
	// 	catch (const std::exception &e)
	// 	{
	// 		std::cerr << "Error: " << e.what() << std::endl;
	// 		return 1;
	// 	}
	// }
	// else
	// {
	// 	try
	// 	{
	// 		Config config;
	// 		std::cout << "Default config created successfully." << std::endl;
	// 		configPrinter(config);
	// 		std::cout << "Default config printed successfully." << std::endl;
	// 		config.checkConfigIsValid();
	// 		std::cout << "Default config is valid." << std::endl;
	// 	}
	// 	catch (const std::exception &e)
	// 	{
	// 		std::cerr << "Error: " << e.what() << std::endl;
	// 		return 1;
	// 	}
	// }
	try
	{
		std::string configPath = (argc == 2) ? argv[1] : "default.conf";
		EventLoop event;
		Config config(configPath);
		ServerSocket socket(*config.getServers().begin());

		event.setStopSignal(&g_shouldStop);
		socket.open(); // Soketi aç, bind ve listen işlemlerini yapsın
		event.addServerSocket(&socket);
		event.run();
	}
	catch (const std::exception &e)
	{
		delete event;
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
