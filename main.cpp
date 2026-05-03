#include "./includes/Config.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char const *argv[])
{
	if(argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	if(argc == 2)
	{
		try
		{
			Config config(argv[1]);
			std::cout << "Config file '" << argv[1] << "' parsed successfully." << std::endl;
			configPrinter(config);
			std::cout << "Config file '" << argv[1] << "' printed successfully." << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
	}
	else
	{
		try
		{
			Config config;
			std::cout << "Default config created successfully." << std::endl;
			configPrinter(config);
			std::cout << "Default config printed successfully." << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
	}
	return 0;
}
