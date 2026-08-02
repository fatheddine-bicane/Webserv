#include <sys/epoll.h>

#include "../Definitions/ClientConnection.hpp"
#include "../../../Responce_builder/Definitions/Responce.hpp"
#include "../../Typedef.hpp"

ClientConnection::ClientConnection(SOCKET fd,
								   String& ip_port,
								   Servers& servers)
	: Connection(CLIENT_S, fd),
	  request(fd, this),
	  ip_port(ip_port),
	  servers(servers),
	  responce(this) {
	this->server = NULL;
}



void	ClientConnection::monitorSockerForOutput(EP_INSTANCE ep_instance) {
	struct epoll_event event;

	// monitor socket for output
    event.events = EPOLLOUT | EPOLLET;
    event.data.ptr = this;

    if (epoll_ctl(ep_instance, EPOLL_CTL_MOD, this->fd, &event) == -1) {
        throw SystemCallsFailedException("epoll_ctl()");
    }
}
