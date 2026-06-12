#include "../Definitions/ServerMultiplexing.hpp"


ServerMultiplexing::ServerMultiplexing(Addresses& addresses, EP_INSTANCE epfd)
: _epfd(epfd), _addresses(addresses) {}


void ServerMultiplexing::bootstrapServerListeners(){
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	Addresses::iterator it = this->_addresses.begin();
	Addresses::iterator end = this->_addresses.end();
	std::vector<Connection*> open_sockets;
	for(;it != end; ++it){
		try{
			SOCKET socket_listen = createListeningSocket(it, &hints);
			monitorListeningSocket(socket_listen, it, open_sockets);
		} catch(SystemCallsFailedException &e){
			std::for_each(open_sockets.begin(), open_sockets.end(), cleanup);
			throw;
		}
	}
}

void ServerMultiplexing::monitorListeningSocket(SOCKET socket_listen, Addresses::iterator& ip_port,
												std::vector<Connection*>& open_sockets){

	ServerConnection *server_connection = new ServerConnection(socket_listen);

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = server_connection;
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, socket_listen, &event) < 0){
		CloseSocket(socket_listen);
		delete server_connection;
		throw SystemCallsFailedException("epoll_ctl()");
	}
	this->_socket_map[socket_listen] = String(ip_port->first + ":" + ip_port->second);
	open_sockets.push_back(server_connection);
}

SOCKET ServerMultiplexing::createListeningSocket(Addresses::iterator& ip_port, struct addrinfo* hints){
	struct	addrinfo* bind_addr = NULL;

	int status = getaddrinfo(ip_port->first.c_str(), ip_port->second.c_str(), hints, &bind_addr);
	if (status != 0)
		throw SystemCallsFailedException("getaddrinfo()");

	SOCKET sock_listen = socket(bind_addr->ai_family, bind_addr->ai_socktype, bind_addr->ai_protocol);
	if (!IsValidSocket(sock_listen))
		throw SystemCallsFailedException("socket()");


	// WARNING: debugging
	// --------------------------------------------------------
	int opt = 1;
	if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		CloseSocket(sock_listen);
		freeaddrinfo(bind_addr);
		throw SystemCallsFailedException("setsockopt(SO_REUSEADDR)");
	}
	// --------------------------------------------------------


	if (bind(sock_listen, bind_addr->ai_addr, bind_addr->ai_addrlen) < 0) {
		CloseSocket(sock_listen);
		throw BindSysCallFailedException(ip_port);
	}

	if(listen(sock_listen, 128) < 0){
		CloseSocket(sock_listen);
		throw SystemCallsFailedException("listen()");
	}

	freeaddrinfo(bind_addr);
	return sock_listen;
}

void ServerMultiplexing::cleanup(Connection* connection) {
	CloseSocket(connection->fd);
	delete connection;
}

SocketsMap& ServerMultiplexing::getSocketsMap(){
	return this->_socket_map;
}
