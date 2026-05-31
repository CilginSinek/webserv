#include "utils/Buffer.hpp"
#include "AConnection.hpp"
#include "ServerSocket.hpp"

typedef enum eConnectionState
{
	READING,
	WRITING
}	tConnectionState;

class ClientConnection: public AConnection
{
private:
	Buffer _readBuffer;
	Buffer _writeBuffer;
	ServerSocket *_serverSocket;
	tConnectionState _state;
	time_t _lastActiveTime;
	ssize_t getRequestSize(const Buffer &buffer) const;
	Buffer generateResponse(const Buffer &request) const;
public:
	ClientConnection();
	ClientConnection(int fd, ServerSocket *serverSocket);
	ClientConnection(const ClientConnection &other);
	ClientConnection &operator=(const ClientConnection &other);
	~ClientConnection();

	void addReadBuffer(const Buffer &buffer);
	const ServerSocket* getServerSocket() const;
	Buffer getWriteBuffer();
	bool isNeedToClose() const;
	tConnectionState getState() const;
};
