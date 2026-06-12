#pragma once

#include "Connection.hpp"

class ServerConnection: public Connection{
public:
	ServerConnection(SOCKET fd) : Connection(SERVER_S, fd){};
};
