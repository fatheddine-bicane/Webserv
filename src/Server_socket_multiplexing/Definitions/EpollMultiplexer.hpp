#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <sys/epoll.h>
#include "Listener.hpp"
#include "ClientConnection.hpp"
#include "../../config_parser/Parser/Definitions/Parser.hpp"

typedef std::string String;
typedef std::map<String, std::map<String, Server> > Servers;

class EpollMultiplexer{
private:
	int _epoll_fd;
	std::map<String, Listener> _listeners;
	std::map<int, ClientConnection> _clients;
	std::map<int, String> _fd_to_endpoint;
	// const Servers* _servers;
	const std::vector<std::pair<String, String> >* _addresses;

	static const int MAX_EVENTS = 64;
	static const int EPOLL_TIMEOUT_MS = 1000;

	int createListeningSocket(const String& ip, const String& port);
	void setSocketNonBlocking(int fd);
	void setSocketReuseAddr(int fd);
	void handleListenerEvent(int listener_fd);
	void handleClientEvent(int client_fd);
	void closeClient(int fd);
public:
	EpollMultiplexer(const std::vector<std::pair<String, String> >*addresses);
	~EpollMultiplexer();

	bool init();
	bool bootstrapListeners();
	void eventLoop();
	void cleanup();
};
