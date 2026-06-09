#pragma once

#include "Connection.hpp"

class ClientConnection : public Connection{
public:
	ClientConnection(SOCKET fd): Connection(CLIENT_S, fd) {};
};
