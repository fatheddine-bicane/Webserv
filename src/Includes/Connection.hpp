#pragma once

typedef int SOCKET;

enum ConnectionType {
	SERVER_S, CLIENT_S
};



class Connection {
public:
	SOCKET			fd;
	ConnectionType	type;

public:
	Connection(ConnectionType type, SOCKET fd) {
		this->type = type;
		this->fd = fd;
	}
};
