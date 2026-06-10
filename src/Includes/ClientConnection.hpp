#pragma once

#include "Connection.hpp"
#include "../HTTP_request_parser/Definitions/Request.hpp"

class ClientConnection : public Connection{
public:
	Request	request;

public:
	ClientConnection(SOCKET fd): Connection(CLIENT_S, fd) {};
};
