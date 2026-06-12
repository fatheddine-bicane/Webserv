#pragma once

#include "Connection.hpp"
#include "../../../HTTP_request_parser/Definitions/Request.hpp"


enum ConnectionState {
	KEEP_ALIVE, CLOSE
};

class ClientConnection : public Connection{
public:
	Request			request;
	ConnectionState	state;

public:
	ClientConnection(SOCKET fd)
	: Connection(CLIENT_S, fd),
	  request(fd, this) {}
};
