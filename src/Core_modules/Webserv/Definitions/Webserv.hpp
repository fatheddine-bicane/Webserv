#pragma once

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../../../Config_parser/Scanner/Definitions/Scanner.hpp"
#include "../../../Server_multiplexing/Definitions/ServerMultiplexing.hpp"
#include "../../../Config_parser/Parser/Definitions/Parser.hpp"
#include "../../../Config_parser/Scanner/Definitions/Token.hpp"
#include "../../../Server_multiplexing/Definitions/ServerMultiplexing.hpp"
#include "../../../Response_builder/Exceptions/ResponseExceptions.hpp"

#include "../../Connection/Definitions/Connection.hpp"
#include "../../Connection/Definitions/ServerConnection.hpp"
#include "../../Connection/Definitions/ClientConnection.hpp"
#include "../../Connection/Exceptions/ConnectionExceptions.hpp"
#include "../../../Process_request/Definitions/ProcessRequest.hpp"
#include "../../Typedef.hpp"

#define MAX_EVENTS 10
#define TIMEOUT -1


class Webserv {
public:
	Servers*	servers;
	SocketsMap*	sockets_map;
	EP_INSTANCE	epfd;
	int					events_size;
	struct epoll_event	events[MAX_EVENTS];
	// log server errors
	std::ofstream error_log;

public:
	// INFO: constructors/destructor
	Webserv();
	~Webserv();

	// INFO: setters
	void	setServers(Servers& servers);
	void	setSocketsMap(SocketsMap& sockets_map);

public:
	//INFO: api
	void	getReadySockets();
	Connection*	getConnectionObject(int index);
	void	addNewClientConnection(Connection* connection);
	void	removeClient(ClientConnection* client_connection);
};
