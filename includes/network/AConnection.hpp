class AConnection
{
protected:
	int _fd;

public:
	AConnection();
	AConnection(int fd);
	AConnection(const AConnection &other);
	AConnection &operator=(const AConnection &other);
	virtual ~AConnection();

	// int				getFd();
	// virtual bool	handleRead();
	// virtual	bool	handleWrite();
	// virtual void	close();
};
