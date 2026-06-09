#pragma once

#include "Connection.hpp"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>

class ServerConnection: public Connection{
public:
	ServerConnection(SOCKET fd) : Connection(SERVER_S, fd){};
};
