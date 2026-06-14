#include "../Definitions/ClientConnection.hpp"

ClientConnection::ClientConnection(SOCKET fd,
								   String& ip_port,
								   Servers& servers)
	: Connection(CLIENT_S, fd),
	  request(fd, this),
	  ip_port(ip_port),
	  servers(servers) {
	this->server = NULL;
}
