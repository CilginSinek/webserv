#include "network/ServerSocket.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // getaddrinfo, freeaddrinfo, gai_strerror, addrinfo
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <string>
#include <sstream>      // std::ostringstream

#include "utils/Utils.hpp"

ServerSocket::ServerSocket(ServerConfig serverConfig) : _fd(-1)
{
    _configs.push_back(serverConfig);
}

ServerSocket::~ServerSocket()
{
    if (_fd != -1)
    {
        ::close(_fd);
    }
}

void ServerSocket::open()
{
    if (_fd != -1)
        return;

    int port = getConfig().getPort();
    std::string host = getConfig().getServerIp();

    std::ostringstream oss;
    oss << port;
    std::string port_str = oss.str();

    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *p = NULL;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;

    const char *host_cstr = NULL;

    if (!host.empty() && host != "0.0.0.0")
        host_cstr = host.c_str();

    int status = getaddrinfo(host_cstr, port_str.c_str(), &hints, &res);
    if (status != 0)
        throw std::runtime_error(std::string("getaddrinfo() failed: ") + gai_strerror(status));

    int saved_errno = 0;

    for (p = res; p != NULL; p = p->ai_next)
    {
        _fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_fd == -1)
        {
            saved_errno = errno;
            continue;
        }

        int opt = 1;
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            saved_errno = errno;
            ::close(_fd);
            _fd = -1;
            continue;
        }

        if (bind(_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            saved_errno = errno;
            ::close(_fd);
            _fd = -1;
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (_fd == -1)
        throw std::runtime_error(std::string("bind/socket failed: ") + std::strerror(saved_errno));

    if (listen(_fd, SOMAXCONN) == -1)
    {
        saved_errno = errno;
        ::close(_fd);
        _fd = -1;
        throw std::runtime_error(std::string("listen() failed: ") + std::strerror(saved_errno));
    }

    setNonBlocking(_fd);
}

int ServerSocket::acceptClient()
{
    if (_fd == -1)
        return -1;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = ::accept(_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &len);
    if (client_fd == -1)
        return -1;

    setNonBlocking(client_fd);
    return client_fd;
}

void ServerSocket::addServerConfig(const ServerConfig &serverConfig)
{
    _configs.push_back(serverConfig);
}

void ServerSocket::close()
{
    if (_fd != -1)
    {
        ::close(_fd);
        _fd = -1;
    }
}

void ServerSocket::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

const ServerConfig &ServerSocket::getConfig() const
{
    return _configs.front();
}

const ServerConfig& ServerSocket::getConfigForHost(const std::string &host) const
{
    std::string normalizedHost = trim(host);
    size_t colonPos = normalizedHost.find(':');
    if (colonPos != std::string::npos)
        normalizedHost = normalizedHost.substr(0, colonPos);
    normalizedHost = upperString(normalizedHost);

    for (std::vector<ServerConfig>::const_iterator it = _configs.begin(); it != _configs.end(); ++it)
    {
        if (upperString(it->getServerName()) == normalizedHost)
            return *it;
    }
    return getConfig();
}

int ServerSocket::getFd()
{
    return (_fd);
}


Session ServerSocket::getAddSession(const std::string &id)
{
    if (id.empty())
    {
        Session newSession(Session::generateId());
        this->_sessions[newSession.getId()] = newSession;
        this->_theIdToSessionId[newSession.getTheId()] = newSession.getId();
        return newSession;
    }
    std::map<std::string, Session>::iterator it = this->_sessions.find(id);
    if (it != this->_sessions.end())
        return it->second;
    Session newSession(id);
    this->_sessions[id] = newSession;
    this->_theIdToSessionId[newSession.getTheId()] = id;
    return newSession;
}

void ServerSocket::cleanSessions()
{
    std::vector<std::string> toRemove;
    time_t currentTime = time(NULL);
    for (std::map<std::string, Session>::iterator it = this->_sessions.begin(); it != this->_sessions.end(); ++it)
    {
        if (difftime(currentTime, it->second.getLastAccessTime()) > 3600)
            toRemove.push_back(it->first);
    }
    for (std::vector<std::string>::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        std::map<std::string, Session>::iterator sit = this->_sessions.find(*it);
        if (sit != this->_sessions.end())
            this->_theIdToSessionId.erase(sit->second.getTheId());
        this->_sessions.erase(*it);
    }
}

void ServerSocket::compareAndSetSession(const Session &session)
{
    std::map<std::string, std::string>::iterator rit = this->_theIdToSessionId.find(session.getTheId());
    if (rit == this->_theIdToSessionId.end())
        return;
    std::string oldSessionId = rit->second;
    if (oldSessionId != session.getId())
    {
        this->_sessions.erase(oldSessionId);
        this->_sessions[session.getId()] = session;
        rit->second = session.getId();
    }
    else
    {
        this->_sessions[oldSessionId] = session;
    }
}