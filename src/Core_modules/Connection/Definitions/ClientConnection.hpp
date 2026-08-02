#pragma once

#include "Connection.hpp"
#include "../../../HTTP_request_parser/Definitions/Request.hpp"
#include "../../../Responce_builder/Definitions/Responce.hpp"
#include "../../../Server_multiplexing/Exceptions/SystemCallsExceptions.hpp"


// class Responce;

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
	Responce	responce;

public:
	ClientConnection(SOCKET fd, String& ip_port, Servers& servers);

	void	monitorSockerForOutput(EP_INSTANCE ep_instance);
};
