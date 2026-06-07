#include "../Definitions/ServerSocketsMultiplexing.hpp"
#include <algorithm>
#include <iostream>
#include <sys/epoll.h>
#include <vector>

// INFO: API
//--------------------------------------------------------------------------------

ServerSocketsMultiplexing::
ServerSocketsMultiplexing(Addresses& addresses, int epfd)
	: _addresses(addresses),
	  _epfd(epfd) {}

//--------------------------------------------------------------------------------

// INFO: API
//--------------------------------------------------------------------------------

void	ServerSocketsMultiplexing::setupServerSockets() {
	Addresses::iterator it = this->_addresses.begin();
	Addresses::iterator end = this->_addresses.end();

	struct addrinfo hints;
	populateHintsStruct(hints);

	std::vector<Connection*> opend_sockets;
	for (; it != end; it++) {
		struct addrinfo* bind_address = NULL;

		try {
			populateBindAddress(it, &hints, &bind_address);

			SOCKET socket_listen = createListeningSocket(bind_address);

			bindListeningSocketToListeningAddress(it, socket_listen, bind_address);

			prepareSocketToAcceptConnections(socket_listen);

			addSocketToEpollInstance(it, socket_listen, opend_sockets);
		} catch (SystemCallsFailedException& e) {
			std::for_each(opend_sockets.begin(), opend_sockets.end(), eraseConnection);
			throw;
		}

	}
}


SocketsMap&	ServerSocketsMultiplexing::getSocketsMap() {
	return this->_sockets_map;
}

//--------------------------------------------------------------------------------



// INFO: utility functions
//--------------------------------------------------------------------------------

void	ServerSocketsMultiplexing::
populateHintsStruct(struct addrinfo& hints) {
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
}



void	ServerSocketsMultiplexing::
populateBindAddress(Addresses::iterator& it,
					struct addrinfo* hints,
					struct addrinfo** bind_address) {

	int status = getaddrinfo(it->first.c_str(), it->second.c_str(), hints, bind_address);
	if (status != 0) {
		throw SystemCallsFailedException("getaddrinfo()");
	}
}


SOCKET	ServerSocketsMultiplexing::
createListeningSocket(struct addrinfo* bind_address) {
	SOCKET socket_listen = socket(bind_address->ai_family,
							      bind_address->ai_socktype,
							      bind_address->ai_protocol);
	if (!IsValidSocket(socket_listen)) {
		throw SystemCallsFailedException("socket()");
	}

	return socket_listen;
}


void	ServerSocketsMultiplexing::
bindListeningSocketToListeningAddress(Addresses::iterator& it,
									  SOCKET socket_listen,
									  struct addrinfo* bind_address) {
	int status = bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen);
	if (status != 0) {
		throw BindSysCallFailedException(it);
	}

	freeaddrinfo(bind_address);
}

void	ServerSocketsMultiplexing::
prepareSocketToAcceptConnections(SOCKET socket_listen) {
	int status = listen(socket_listen, 10);

	if (status != 0) {
		throw SystemCallsFailedException("listen()");
	}

}


void	ServerSocketsMultiplexing::
addSocketToEpollInstance(Addresses::iterator& it,
						 SOCKET& socket_listen,
						 std::vector<Connection*>& opend_sockets) {
	ServerConnection* server_connection = new ServerConnection(socket_listen);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = server_connection;

	int status = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, socket_listen, &event);
	if (status == -1) {
		delete server_connection;
		CloseSocket(socket_listen);
		throw SystemCallsFailedException("epoll_ctl()");
	}

	opend_sockets.push_back(server_connection);
	this->_sockets_map[socket_listen] = String(it->first + ":" + it->second);
}


void	ServerSocketsMultiplexing::eraseConnection(Connection* connection) {
	CloseSocket(connection->fd);
	delete connection;
}

//--------------------------------------------------------------------------------
