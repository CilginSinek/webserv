#include "utils/Buffer.hpp"


Buffer::Buffer() {}

Buffer::Buffer(const std::string &str) : std::string(str) {}

Buffer &Buffer::operator=(const std::string &str)
{
	std::string::operator=(str);
	return *this;
}

Buffer &Buffer::operator=(const Buffer &other)
{
	std::string::operator=(other);
	return *this;
}

Buffer::~Buffer() {}

