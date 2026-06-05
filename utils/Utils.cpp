#include "utils/Utils.hpp"

size_t ft_stoi(const std::string &str)
{
	size_t res = 0;
	size_t i = 0;
	while (i < str.size())
	{
		char c = str[i];
		if (c < '0' || c > '9')
			throw std::invalid_argument("Expected integer value but got '" + str + "'");
		res = res * 10 + (c - '0');
		i++;
	}
	return res;
}

std::string ft_itos(size_t num)
{
	if (num == 0)
		return "0";
	std::string res;
	while (num > 0)
	{
		char c = '0' + (num % 10);
		res = c + res;
		num /= 10;
	}
	return res;
}

bool isIgnoredLine(const std::string &line)
{
	std::string trimmed = line;
	trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
	trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
	return trimmed.empty() || trimmed[0] == '#';
}

std::string upperString(const std::string &str)
{
	std::string res = str;
	for (size_t i = 0; i < res.size(); i++)
		res[i] = std::toupper(res[i]);
	return res;
}

bool endsWith(const std::string &str, const std::string &ext)
{
	if (str.size() < ext.size())
		return false;
	return str.compare(str.size() - ext.size(), ext.size(), ext) == 0;
}

std::string trim(const std::string &str)
{
	size_t start = str.find_first_not_of(". \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(". \t\r\n");
	return str.substr(start, end - start + 1);
}

std::string normalizeEnv(const std::string &str)
{
	std::string res = str;
	for (size_t i = 0; i < res.size(); i++)
	{
		if (res[i] == '-')
			res[i] = '_';
		else
			res[i] = std::toupper(res[i]);
	}
	return trim(res);
}

void debugLogger(const std::string &message)
{
	time_t now = time(0);
	struct tm *ltm = localtime(&now);
	std::cout << "\033[1;34m"; // Set text color to blue
	std::cout << "[" << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday << " " << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << "] ";
	std::cout << "DEBUG: " << message << "\033[0m" << std::endl;
}
