#include "../Definitions/ClientConnection.hpp"

static std::string ipToString(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	std::stringstream ss;
	ss << (int)a << "." << (int)b << "." << (int)c << "." << (int)d;
	return ss.str();
}

ClientConnection::ClientConnection() : _fd(-1), _recv_buffer("") {
	memset(&_client_addr, 0, sizeof(_client_addr));
}

ClientConnection::ClientConnection(int socket_fd, const struct sockaddr_in& addr)
: _fd(socket_fd), _recv_buffer(""), _client_addr(addr) {}

int ClientConnection::getFd() const {
	return _fd;
}

const std::string& ClientConnection::getRecvBuffer() const {
	return _recv_buffer;
}

struct sockaddr_in ClientConnection::getClientAddr() const {
	return _client_addr;
}

std::string ClientConnection::getClientIpAndPort() const {
	unsigned char* ip_bytes = (unsigned char*)&_client_addr.sin_addr.s_addr;
	std::string ip_str = ipToString(ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);

	int port = ntohs(_client_addr.sin_port);

	std::stringstream ss;
	ss << ip_str << ":" << port;
	return ss.str();
}

void ClientConnection::appendToRecvBuffer(const std::string& data) {
	_recv_buffer += data;
}

void ClientConnection::clearRecvBuffer() {
	_recv_buffer.clear();
}


void ClientConnection::printReceivedData() const {
	std::cout << "=== CLIENT DATA FROM " << getClientIpAndPort() << " ===" << std::endl;
	std::cout << _recv_buffer << std::endl;
	std::cout << "=== END ===" << std::endl;
}
