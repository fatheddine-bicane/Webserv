#include "../Definitions/Webserv.hpp"

// INFO: constructors/destructor
// -------------------------------------------------
Webserv::Webserv()
	: error_log("Log/error_log.txt") {
	this->servers = NULL;
	this->sockets_map = NULL;
	this->epfd = epoll_create(1);
	this->events_size = 0;
}


Webserv::~Webserv() {
	delete this->servers;
	delete this->sockets_map;
	CloseSocket(this->epfd);
	this->error_log.close();
}

// -------------------------------------------------


// INFO: setters
// -------------------------------------------------
void	Webserv::setServers(Servers& servers) {
	this->servers = new Servers(servers);
}


void	Webserv::setSocketsMap(SocketsMap& sockets_map) {
	this->sockets_map = new SocketsMap(sockets_map);
}
// -------------------------------------------------


//INFO: api
// -------------------------------------------------

void	Webserv::getReadySockets() {
	this->events_size = epoll_wait(this->epfd, this->events, MAX_EVENTS, TIMEOUT);
}


Connection*	Webserv::getConnectionObject(int index) {
	return (static_cast<Connection*>(this->events[index].data.ptr));
}


void	Webserv::addNewClientConnection(Connection* connection) {
	SOCKET client_socket = accept(connection->fd, NULL, NULL);
	if (!IsValidSocket(client_socket)) {
		throw ConnectionException("accept()");
	}

	int status = fcntl(client_socket, F_SETFL, O_NONBLOCK);
    if (status == -1) {
		CloseSocket(client_socket);
		throw SystemCallsFailedException("fcntl()");
    }

	String& ip_port = this->sockets_map->at(connection->fd);
	ClientConnection* client_connection = new ClientConnection(client_socket,
															   ip_port,
															   *this->servers);
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = client_connection;

	status = epoll_ctl(this->epfd, EPOLL_CTL_ADD, client_socket, &event);
	if (!SocketAdded(status)) {
		CloseSocket(client_socket);
		delete client_connection;
		throw ConnectionException("epoll_ctl()");
	}
}

// -------------------------------------------------
