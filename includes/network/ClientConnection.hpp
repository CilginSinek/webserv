#include "utils/Buffer.hpp"
#include "AConnection.hpp"
#include "ServerSocket.hpp"
#include "parser/RequestParse.hpp"
#include "parser/ResponseParse.hpp"
#include "utils/Utils.hpp"

typedef enum eConnectionState
{
	READING,
	HANDLING,
	WRITING_HEADER,
	WRITING_BODY
}	tConnectionState;

typedef struct sReqData
{
	std::string header;
	std::string bodyFilePath;
	size_t bodySize;
	size_t readedBodySize;
	bool isChunked;
	bool complete;
} tReqData;

class ClientConnection: public AConnection
{
private:
	Buffer _readBuffer;
	Buffer _writeBuffer;
	ServerSocket *_serverSocket;
	tConnectionState _state;
	//* debug
	ssize_t responseCount;
	time_t _lastActiveTime;
	std::queue<tReqData> _requestDataList;
	std::queue<ResponseParse> _responseDataList;
	ssize_t getRequestSize(const Buffer &buffer) const;
	size_t getRequestBodySize(const Buffer &buffer) const;

public:
	ClientConnection();
	ClientConnection(int fd, ServerSocket *serverSocket);
	ClientConnection(const ClientConnection &other);
	ClientConnection &operator=(const ClientConnection &other);
	~ClientConnection();

	void addReadBuffer(const Buffer &buffer);
	const ServerSocket* getServerSocket() const;

	void appendRequestData(const tReqData &reqData);
	tReqData getCurrentRequestData() const;
	void popCurrentRequestData();
	bool requestDataEmpty() const;

	ResponseParse &getCurrentResponseData();
	void appendResponseData(const ResponseParse &responseData);
	void popCurrentResponseData();
	bool responseDataEmpty() const;

	void handleRead();
	void addWriteBuffer(const Buffer &buffer);
	void setWriteBuffer(const Buffer &buffer);
	void setState(tConnectionState state);
	const Buffer &getWriteBuffer();
	bool isNeedToClose() const;
	tConnectionState getState() const;

	//* debug
	void setResponseCount(ssize_t count);
	ssize_t getResponseCount() const;
};
