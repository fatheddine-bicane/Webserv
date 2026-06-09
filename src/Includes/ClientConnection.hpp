#pragma once

#include "Connection.hpp"
#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>

class ClientConnection : public Connection{
public:
	ClientConnection(SOCKET fd): Connection(CLIENT_S, fd) {};
};
