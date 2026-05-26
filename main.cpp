#include "parser/Config.hpp"
#include <iostream>
#include "core/EventLoop.hpp"
#include <stdexcept>

int main(int argc, char const *argv[])
{
	if(argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes when writing to closed sockets
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
		EventLoop *event = new EventLoop();
		Config _config(argv[1]);
		ServerSocket *socket = new ServerSocket(*_config.getServers().begin());
		socket->open(); // Soketi aç, bind ve listen işlemlerini yapsın
		event->addServerSocket(socket);
		event->run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	return 0;
}
