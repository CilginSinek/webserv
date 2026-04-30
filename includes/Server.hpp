#ifndef _SERVER_HPP_
#define _SERVER_HPP_

class Server
{
private:
	int port;
	std::string server_name;
// error_page 404 /404.html; needed to be implemented (which data type should be used for this? map<int, string> ?)
	int keepalive_timeout;
	std::vector<Route> routes;
public:
	Server();
	~Server();
};

#endif