#include "Buffer.hpp"



class ClientConnection
{
private:
	int _fd;
	int _serverFd;
	Buffer _readBuffer;
	Buffer _writeBuffer;
	

public:
	ClientConnection(/* args */);
	~ClientConnection();
};
