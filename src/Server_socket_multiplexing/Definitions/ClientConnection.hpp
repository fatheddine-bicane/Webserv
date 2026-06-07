#pragma once

#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>

class ClientConnection {
private:
	int _fd;
	std::string _recv_buffer;
	struct sockaddr_in _client_addr;
public:
	ClientConnection();
	ClientConnection(int socket_fd, const struct sockaddr_in& addr);

	int getFd() const;
	const std::string& getRecvBuffer() const;
	struct sockaddr_in getClientAddr() const;
	std::string getClientIpAndPort() const;

	void appendToRecvBuffer(const std::string& data);
	void clearRecvBuffer();

	void printReceivedData() const;
};
