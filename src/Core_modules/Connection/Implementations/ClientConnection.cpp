#include "../Definitions/ClientConnection.hpp"

ClientConnection::ClientConnection(SOCKET fd)
	: Connection(CLIENT_S, fd),
	  request(fd, this) { }
