#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <iostream>
#include <cstring>

class Buffer : public std::string
{
public:
	Buffer();
	Buffer(const std::string &str);
	Buffer &operator=(const std::string &str);
	Buffer &operator=(const Buffer &other);
	~Buffer();
};

#endif