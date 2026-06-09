#include "Includes/Webserv.hpp"

// INFO: constructors/destructor
// -------------------------------------------------
Webserv::Webserv()
	: error_log("Log/error_log.txt") {
	this->servers = NULL;
	this->sockets_map = NULL;
	this->epfd = epoll_create(1);
}


Webserv::~Webserv() {
	delete this->servers;
	delete this->sockets_map;
	CloseSocket(this->epfd);
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
