#include "../Definitions/Listener.hpp"

Listener::Listener() : _fd(-1), _ip(""), _port("") {
	memset(&_bind_addr, 0, sizeof(_bind_addr));
}

Listener::Listener(int socket_fd, const std::string& ip_addr, const std::string& port_num)
: _fd(socket_fd), _ip(ip_addr), _port(port_num) {

	memset(&_bind_addr, 0, sizeof(_bind_addr));

	struct addrinfo hints;
	struct addrinfo* result = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(ip_addr.c_str(), port_num.c_str(), &hints, &result);

	if (status == 0 && result != NULL) {
		memcpy(&_bind_addr, result->ai_addr, result->ai_addrlen);
		freeaddrinfo(result);
	}
}

int Listener::getFd() const {
	return _fd;
}

const std::string& Listener::getIp() const {
	return _ip;
}

const std::string& Listener::getPort() const {
	return _port;
}

struct sockaddr_in Listener::getBindAddr() const {
	return _bind_addr;
}

void Listener::printInfo() const {
	std::cout << "[Listener] fd=" << _fd << " listening on " 
		<< _ip << ":" << _port << std::endl;
}
