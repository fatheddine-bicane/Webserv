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

#include "../../Includes/ServerConnection.hpp"
#include "../Exceptions/SystemCallsExceptions.hpp"
#include "../../Includes/Typedef.hpp"


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

	void	bootstrapServerListeners();
	SocketsMap&	getSocketsMap();
};
