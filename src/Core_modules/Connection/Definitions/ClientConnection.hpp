#pragma once

#include "Connection.hpp"
#include "../../../HTTP_request_parser/Definitions/Request.hpp"
#include "../../../HTTP_response_processer/Definitions/Response.hpp"
#include "../../../Server_multiplexing/Exceptions/SystemCallsExceptions.hpp"


enum ConnectionState {
	KEEP_ALIVE, CLOSE
};

class ClientConnection : public Connection{
public:
	Request			request;
	ConnectionState	state;
	Server*			server;
	String&			ip_port;
	Servers&		servers;
	Response		response;

	// if the request is cgi
	pid_t			pid;
	File			pipe_read_end;

public:
	ClientConnection(SOCKET fd, String& ip_port, Servers& servers);

	void	monitorSockerForOutput(EP_INSTANCE ep_instance);
};
