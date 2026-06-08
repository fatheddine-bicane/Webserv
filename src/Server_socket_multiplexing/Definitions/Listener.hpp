#pragma once

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>

class Listener{
private:
	int _fd;
	std::string _ip;
	std::string _port;
	struct sockaddr_in _bind_addr;
public:
	Listener();
	Listener(int socket_fd, const std::string& ip_addr, const std::string& port_num);

	int getFd() const;
	const std::string& getIp() const;
	const std::string& getPort() const;
	struct sockaddr_in getBindAddr() const;

	void printInfo() const;
};
