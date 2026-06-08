#include "../Definitions/EpollMultiplexer.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


static std::string inet_address_to_string(const struct sockaddr_in& addr) {
	unsigned char* ip_bytes = (unsigned char*)&addr.sin_addr.s_addr;
	int port = ntohs(addr.sin_port);

	std::stringstream ss;
	ss << (int)ip_bytes[0] << "." << (int)ip_bytes[1] << "." 
		<< (int)ip_bytes[2] << "." << (int)ip_bytes[3] << ":" << port;
	return ss.str();
}

EpollMultiplexer::EpollMultiplexer(const std::vector<std::pair<String, String> >* addresses)
: _epoll_fd(-1), _addresses(addresses) {}

EpollMultiplexer::~EpollMultiplexer() {
	cleanup();
}

bool EpollMultiplexer::init(){
	_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if(_epoll_fd < 0){
		std::cerr << "epoll_create1() failed: " << strerror(errno) << std::endl;
		return false;
	}
	std::cout << "[EPOLL] Created epoll instance (fd=" << _epoll_fd << ")" << std::endl;
	if (!bootstrapListeners()) {
		std::cerr << "[ERROR] Failed to bootstrap listeners" << std::endl;
		return false;
	}
	return true;
}

bool EpollMultiplexer::bootstrapListeners(){
	if (_addresses == NULL || _addresses->empty()){
		std::cerr << "[ERROR] no addresses to listen on" << std::endl;
		return false;
	}

	std::set<String> unique_endpoints;

	for(size_t i = 0; i < _addresses->size(); i++){
		String ip = (*_addresses)[i].first;
		String port = (*_addresses)[i].second;
		String endpoint = ip + ":" + port;
		unique_endpoints.insert(endpoint);
	}
	std::cout << "[BOOTSTRAP] Found " << unique_endpoints.size() 
	          << " unique endpoint(s)" << std::endl;

	for(std::set<String>::iterator it = unique_endpoints.begin(); it != unique_endpoints.end(); ++it){
		String endpoint = *it;
		size_t colon_pos = endpoint.rfind(':');
		if (colon_pos == String::npos) {
			std::cerr << "[ERROR] Invalid endpoint format: " << endpoint << std::endl;
			continue;
		}
		String ip = endpoint.substr(0, colon_pos);
		String port = endpoint.substr(colon_pos+1);
		std::cout << "[BOOTSTRAP] Creating listener for " << endpoint << std::endl;
		int listener_fd = createListeningSocket(ip, port);
		if (listener_fd < 0){
			std::cerr << "[ERROR] Failed to create listening socket for " << endpoint << std::endl;
			continue;
		}
		Listener listener(listener_fd, ip, port);
		_listeners[endpoint] = listener;
		_fd_to_endpoint[listener_fd] = endpoint;

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = listener_fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, listener_fd, &ev) < 0){
			std::cerr << "[ERROR] epoll_ctl(ADD) failed for fd=" << listener_fd 
			          << ": " << strerror(errno) << std::endl;
			close(listener_fd);
			_listeners.erase(endpoint);
			_fd_to_endpoint.erase(listener_fd);
			continue;
		}
		std::cout << "[BOOTSTRAP] Listening on " << endpoint 
		          << " (fd=" << listener_fd << ")" << std::endl;
	}
	if (_listeners.empty()){
		std::cerr << "[ERROR] No listeners successfully created" << std::endl;
		return false;
	}
	return true;
}

int EpollMultiplexer::createListeningSocket(const String& ip, const String& port){
	struct addrinfo hints;
	struct addrinfo* result = NULL;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
	if (status != 0){
		std::cerr << "[ERROR] getaddrinfo() failed: " << gai_strerror(status) << std::endl;
		return -1;
	}
	if (result == NULL){
		std::cerr << "[ERROR] getaddrinfo() returned NULL" << std::endl;
		return -1;
	}

	int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock_fd < 0) {
		std::cerr << "[ERROR] socket() failed: " << strerror(errno) << std::endl;
		freeaddrinfo(result);
		return -1;
	}

	setSocketReuseAddr(sock_fd);
	setSocketNonBlocking(sock_fd);

	if (bind(sock_fd, result->ai_addr, result->ai_addrlen) < 0) {
		std::cerr << "[ERROR] bind() failed on " << ip << ":" << port 
			<< ": " << strerror(errno) << std::endl;
		close(sock_fd);
		freeaddrinfo(result);
		return -1;
	}

	if(listen(sock_fd, 128) < 0){
		std::cerr << "[ERROR] listen() failed: " << strerror(errno) << std::endl;
		close(sock_fd);
		freeaddrinfo(result);
		return -1;
	}
	freeaddrinfo(result);
	return sock_fd;
}

void EpollMultiplexer::setSocketReuseAddr(int fd) {
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cerr << "[WARN] setsockopt(SO_REUSEADDR) failed: " 
			<< strerror(errno) << std::endl;
	}
}

void EpollMultiplexer::setSocketNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		std::cerr << "[ERROR] fcntl(F_GETFL) failed: " << strerror(errno) << std::endl;
		return;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		std::cerr << "[ERROR] fcntl(F_SETFL, O_NONBLOCK) failed: " 
			<< strerror(errno) << std::endl;
	}
}

void EpollMultiplexer::eventLoop(){
	std::cout << "[EVENT_LOOP] Starting main event loop..." << std::endl;
	struct epoll_event events[MAX_EVENTS];
	while (true){
		int num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
		if (num_events < 0){
			if (errno == EINTR) {
				continue;  // Interrupted by signal, retry
			}
			std::cerr << "[ERROR] epoll_wait() failed: " << strerror(errno) << std::endl;
			break;
		}
		if (num_events == 0)
			continue;
		for (int i = 0; i < num_events; i++){
			int fd = events[i].data.fd;
			int event_mask = events[i].events;

			if (_fd_to_endpoint.find(fd) != _fd_to_endpoint.end())
				handleListenerEvent(fd);
			else if(_clients.find(fd) != _clients.end()){
				if (event_mask & EPOLLIN)
					handleClientEvent(fd);
				if (event_mask & (EPOLLERR | EPOLLHUP))
					closeClient(fd);
			}
		}
	}
	std::cout << "[EVENT_LOOP] Exiting event loop" << std::endl;
}

void EpollMultiplexer::handleListenerEvent(int listener_fd){
	while (true){
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);

		int client_fd = accept(listener_fd, (struct sockaddr*)&client_addr, &addr_len);

		if (client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}
			std::cerr << "[ERROR] accept() failed: " << strerror(errno) << std::endl;
			break;
		}
		
		std::cout << "[ACCEPT] New connection from " 
		          << inet_address_to_string(client_addr) << " (fd=" << client_fd << ")" << std::endl;

		ClientConnection client(client_fd, client_addr);
		_clients[client_fd] = client;

		setSocketNonBlocking(client_fd);

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = client_fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
			std::cerr << "[ERROR] epoll_ctl(ADD) failed for client fd=" << client_fd 
			          << ": " << strerror(errno) << std::endl;
			close(client_fd);
			_clients.erase(client_fd);
		}
	}
}

void EpollMultiplexer::handleClientEvent(int client_fd){
	char buffer[4096];
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return;
		}
		std::cerr << "[ERROR] recv() failed: " << strerror(errno) << std::endl;
		closeClient(client_fd);
		return;
	}
	if (bytes_received == 0){
		std::cout << "[CLOSE] Client closed connection (fd=" << client_fd << ")" << std::endl;
		closeClient(client_fd);
		return;
	}
	_clients[client_fd].appendToRecvBuffer(buffer);
	_clients[client_fd].printReceivedData();
}

void EpollMultiplexer::closeClient(int fd) {
	std::cout << "[CLOSE] Closing client connection (fd=" << fd << ")" << std::endl;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
		std::cerr << "[WARN] epoll_ctl(DEL) failed: " << strerror(errno) << std::endl;
	}

	if (close(fd) < 0) {
		std::cerr << "[WARN] close() failed: " << strerror(errno) << std::endl;
	}

	_clients.erase(fd);
}

void EpollMultiplexer::cleanup() {
	std::cout << "[CLEANUP] Cleaning up EpollMultiplexer..." << std::endl;

	std::map<int, ClientConnection>::iterator client_it = _clients.begin();
	while (client_it != _clients.end()) {
		int fd = client_it->first;
		close(fd);
		++client_it;
	}
	_clients.clear();

	std::map<String, Listener>::iterator listener_it = _listeners.begin();
	while (listener_it != _listeners.end()) {
		int fd = listener_it->second.getFd();
		close(fd);
		++listener_it;
	}
	_listeners.clear();
	_fd_to_endpoint.clear();

	if (_epoll_fd >= 0) {
		close(_epoll_fd);
	}

	std::cout << "[CLEANUP] Cleanup complete" << std::endl;
}
