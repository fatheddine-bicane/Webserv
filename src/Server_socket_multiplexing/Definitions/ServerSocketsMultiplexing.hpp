# pragma once

#include <map>
#include <netdb.h>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <utility>
#include <unistd.h>
#include "../Exceptions/SystemCallsException.hpp"
#include "../../Includes/ServerConnection.hpp"

#define IsValidSocket(s) ((s) >= 0)
#define CloseSocket(s) (close(s))

typedef std::string String;
typedef std::vector<std::pair<String, String> > Addresses;
typedef std::map<int, String> SocketsMap;


class ServerSocketsMultiplexing {
private:
	SocketsMap	_sockets_map;
	Addresses&	_addresses;
	int			_epfd;


// INFO: constructor
public:
	ServerSocketsMultiplexing(Addresses& addresses, int epfd);


// INFO: API
public:
	void	setupServerSockets();
	SocketsMap&	getSocketsMap();


// INFO: utility functions
private:

	void	populateHintsStruct(struct addrinfo& hints);
	void	populateBindAddress(Addresses::iterator& it,
							    struct addrinfo* hints,
							    struct addrinfo** bind_address);
	SOCKET	createListeningSocket(struct addrinfo* bind_address);
	void	bindListeningSocketToListeningAddress(Addresses::iterator& it,
												  SOCKET socket_listen,
												  struct addrinfo* bind_address);
	void	prepareSocketToAcceptConnections(SOCKET socket_listen);
	void	addSocketToEpollInstance(Addresses::iterator& it,
								     SOCKET& socket_listen,
								     std::vector<Connection*>& opend_sockets);
	static void	eraseConnection(Connection* connection);
};
