<i>This project has been created as part of the 42 curriculum by iduman, vzeybek</i>

# Webserv

## Description

Webserv is a simple HTTP/1.1 Web Server implemented in C++. It supports basic HTTP methods such as GET, POST, and DELETE, and can serve static files as well as execute CGI scripts.

Server uses non-blocking I/O and the `poll` system call to handle multiple client connections concurrently. It also includes a configuration parser that allows users to specify server settings such as port number, root directory, and error pages.

## Instructions

### Installation
1. Clone the repository in github or any other git link:
```bash
git clone https://github.com/CilginSinek/webserv webserv
```
2. Navigate to the project directory:
```bash
cd webserv
```
3. Build the project using the Makefile:
```bash
make
```

### Usage

1. Run the web server:
```bash
./webserv <config-file>
```
Replace `<config-file>` with the path to your configuration file.

## Resources

### Documentations
- HTTP/1.1 Documentation: [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616)
- HTTP/1.1 Status Codes: [MDN Web Docs](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)
- HTTP/1.1 Methods: [RFC 2616 Section 5.1.1](https://datatracker.ietf.org/doc/html/rfc2616#section-5.1.1)
- CGI Documentation: [CGI Specification](https://www.ietf.org/rfc/rfc3875)
- poll, select, epoll and etc functions: [Linux Manual Pages]

### Tools
- Valgrind: [Valgrind Official Website](https://valgrind.org/)
- GDB: [GDB Official Website](https://www.gnu.org/software/gdb/)
- Siege: [Siege Manual](https://linux.die.net/man/1/siege)
- Curl: [Curl Manual](https://curl.se/docs/manpage.html)

### use of AI

It was used to identify the causes of undefined behaviors more quickly and to obtain recommendations for achieving an optimal structure in the project architecture. And used in git action to split terminals into multiple sections to run server and tester at the same time.
