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
	Server*			server;
	String&	ip_port;
	Servers&	servers;

public:
	ClientConnection(SOCKET fd, String& ip_port, Servers& servers);
};
