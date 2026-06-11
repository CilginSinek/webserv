#include "network/Session.hpp"

Session::Session(): _the_id(generateId()), _id(""), _lastAccessTime(std::time(NULL))
{
}

Session::Session(const std::string &id): _the_id(generateId()), _id(id), _lastAccessTime(std::time(NULL))
{
}

Session::Session(const Session &other)
{
	*this = other;
}

Session &Session::operator=(const Session &other)
{
	if (this != &other)
	{
		this->_the_id = other._the_id;
		this->_id = other._id;
		this->_data = other._data;
		this->_lastAccessTime = other._lastAccessTime;
	}
	return *this;
}

Session::~Session()
{
}

std::string Session::generateId()
{
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string id;
	for (int i = 0; i < 32; ++i)
	{
		id += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return id;
}

const std::string &Session::getId() const
{
	return this->_id;
}

const std::string &Session::getTheId() const
{
	return this->_the_id;
}

void Session::setData(const std::string &key, const std::string &value)
{
	this->_data[key] = value;
}

const std::map<std::string, std::string> &Session::getAllData() const
{
	return this->_data;
}

std::time_t Session::getLastAccessTime() const
{
	return this->_lastAccessTime;
}

void Session::updateLastAccessTime()
{
	this->_lastAccessTime = std::time(NULL);
}