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
#include "../../../HTTP_response_processer/Exceptions/ResponseExceptions.hpp"

#include "../../Connection/Definitions/Connection.hpp"
#include "../../Connection/Definitions/ServerConnection.hpp"
#include "../../Connection/Definitions/ClientConnection.hpp"
#include "../../Connection/Exceptions/ConnectionExceptions.hpp"
#include "../../../HTTP_request_processer/Definitions/ProcessRequest.hpp"
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
	std::vector<pid_t>	cgis_to_reap;
	std::vector<Connection*>	server_sockets;
	std::map<SOCKET, ClientConnection*> client_sockets;
	static sig_atomic_t	signal_status;

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
	void	reapCGIUnfinishedProcesses();
	static void	catch_sigint(sig_atomic_t signum);
	static void cleanup_server(Connection* connection);
	static void cleanup_client(
				const std::pair<SOCKET, ClientConnection*>& client);
	bool	isServerInterupted();
};
