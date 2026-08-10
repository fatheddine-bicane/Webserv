#pragma once

#include "Connection.hpp"
#include "../../../HTTP_request_parser/Definitions/Request.hpp"
#include "../../../HTTP_response_processer/Definitions/Response.hpp"
#include "../../../Server_multiplexing/Exceptions/SystemCallsExceptions.hpp"
#include <sys/_types/_pid_t.h>


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

	// if the request is cgi to prevent zombie child process
	pid_t			pid;

public:
	ClientConnection(SOCKET fd, String& ip_port, Servers& servers);

	void	monitorSockerForOutput(EP_INSTANCE ep_instance);
};
