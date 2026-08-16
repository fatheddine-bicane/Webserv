#pragma once

#include <map>
#include <netdb.h>
#include <set>
#include <vector>
#include <sys/epoll.h>
#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include "../../Core_modules/Connection/Definitions/ServerConnection.hpp"
#include "../Exceptions/SystemCallsExceptions.hpp"
#include "../../Core_modules/Typedef.hpp"


class ServerMultiplexing{
private:
	EP_INSTANCE	_epfd;
	SocketsMap	_socket_map;
	Addresses&	_addresses;

	SOCKET createListeningSocket(Addresses::iterator& ip_port, struct addrinfo* hints);
	void monitorListeningSocket(SOCKET socket_listen, Addresses::iterator& ip_port,
							 std::vector<Connection*>& open_sockets);
	static void cleanup(Connection* connection);
public:
	ServerMultiplexing(Addresses& addresses, EP_INSTANCE epfd);

	std::vector<Connection*>	bootstrapServerListeners();
	SocketsMap&	getSocketsMap();
};
