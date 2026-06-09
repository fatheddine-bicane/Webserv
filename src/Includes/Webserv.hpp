#pragma once

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../config_parser/Scanner/Definitions/Scanner.hpp"
#include "../Server_socket_multiplexing/Definitions/ServerMultiplexing.hpp"
#include "../config_parser/Parser/Definitions/Parser.hpp"
#include "../config_parser/Scanner/Definitions/Token.hpp"
#include "../Server_socket_multiplexing/Definitions/ServerMultiplexing.hpp"
#include "Connection.hpp"
#include "ServerConnection.hpp"
#include "ClientConnection.hpp"
#include "Typedef.hpp"


class Webserv {
public:
	Servers*	servers;
	SocketsMap*	sockets_map;
	EP_INSTANCE	epfd;

public:
	// INFO: constructors/destructor
	Webserv();
	~Webserv();

	// INFO: setters
	void	setServers(Servers& servers);
	void	setSocketsMap(SocketsMap& sockets_map);
};
