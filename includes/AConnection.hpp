class AConnection
{
private:
	int _fd;

public:
	AConnection(/* args */);
	virtual ~AConnection();

	int				getFd();
	virtual bool	handleRead();
	virtual	bool	handleWrite();
	virtual void	close();
};
