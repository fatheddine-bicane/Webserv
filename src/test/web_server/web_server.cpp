#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <list>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

typedef int SOCKET;
#define IsValidSocket(s) ((s) >= 0)
#define CloseSocket(s) (close(s))


#include <iostream>
#include <stdlib.h>

// hardcoded version of 'file --mime-type [filename]'
std::string	getContentType(const std::string& path) {
	size_t dot_pos = path.find_last_of('.');

	if (dot_pos != std::string::npos) {
		std::string extention = path.substr(dot_pos);
		if (extention == ".css") return "text/css";
		if (extention == ".csv") return "text/csv";
		if (extention == ".gif") return "image/gif";
		if (extention == ".htm") return "text/html";
		if (extention == ".html") return "text/html";
		if (extention == ".ico") return "image/x-icon";
		if (extention == ".jpeg") return "image/jpeg";
		if (extention == ".jpg") return "image/jpeg";
		if (extention == ".js") return "application/javascript";
		if (extention == ".json") return "application/json";
		if (extention == ".png") return "image/png";
		if (extention == ".pdf") return "application/pdf";
		if (extention == ".svg") return "image/svg+xml";
		if (extention == ".txt") return "text/plain";
	}

	return "application/octet-stream";
}

SOCKET	createSocket(const std::string& host, const std::string& port) {
	std::cout << "Configuring local address...\n";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* bind_address;
	if (host == "") {
		int status = getaddrinfo(NULL, port.c_str(), &hints, &bind_address);
	} else {
		int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &bind_address);
	}

	std::cout << "Creating socket...\n";
	SOCKET socket_listen = socket(bind_address->ai_family,
								  bind_address->ai_socktype,
								  bind_address->ai_protocol);
	if (!IsValidSocket(socket_listen)) {
		std::cerr << "socket() failed. (" << errno << ")\n";
		exit(1);
	}


	// INFO: this code tels the kernel its ok to reuse the port even if the socket 
	// binded to that port is in TIME_WAIT state
	struct linger sl;
	sl.l_onoff = 1;      // Enable linger
	sl.l_linger = 0;     // Timeout 0 seconds (Hard Close)
	int opt = 1;

	// 1. Allow immediate rebinding
	setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	// 2. Force a "Hard Close" on exit to purge old packets
	sl.l_onoff = 1; 
	sl.l_linger = 0; 
	setsockopt(socket_listen, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));

	// if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	// 	std::cerr << "setsockopt() failed. (" << errno << ")\n";
	// 	CloseSocket(socket_listen);
	//        exit(1);
	//    }
	// ---------------------------------------------------------------------

	std::cout << "Binding to local address...\n";
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
		std::cerr << "bind() failed. (" << errno << ")\n";
		exit(1);
	}
	freeaddrinfo(bind_address);

	std::cout << "Listening...\n";
	if (listen(socket_listen, 10) < 0) {
		std::cerr << "listen() failed. (" << errno << ")\n";
		exit(1);
	}

	return socket_listen;
}


#define MAX_REQUEST_SIZE 2047


class ClientInfo {
public:
	socklen_t	address_length;
	struct sockaddr_storage	address;
	SOCKET	socket;
	char request[MAX_REQUEST_SIZE + 1];
	int received;

};


ClientInfo&	getClient(SOCKET s, std::list<ClientInfo>& clients) {
	std::list<ClientInfo>::iterator it = clients.begin();
	std::list<ClientInfo>::iterator end = clients.end();
	for (; it != end; it++) {
		if (it->socket == s) return *it;
	}

	ClientInfo new_client;
	new_client.received = 0;
	new_client.received = 0;
	new_client.address_length = sizeof(new_client.address);
	clients.push_back(new_client);
	return clients.back();
}

std::list<ClientInfo>::iterator	dropClient(ClientInfo& client, std::list<ClientInfo>& clients) {
	CloseSocket(client.socket);

	std::list<ClientInfo>::iterator it = clients.begin();
	std::list<ClientInfo>::iterator end = clients.end();
	for (; it != end; it++) {
		// if (&client == &(*it)) { }
		if (client.socket == it->socket) {
			return clients.erase(it);
		}
	}

	std::cerr << "dropClient not found.\n";
	exit(1);
}


std::string getClientAddress(ClientInfo& client) {
	char buffer[100];
	getnameinfo((struct sockaddr*)&client.address, client.address_length,
			 buffer, 100,
			 NULL, 0, NI_NUMERICHOST);
	return buffer;
}

fd_set waitOnClients(SOCKET server, std::list<ClientInfo>& clients) {
	fd_set reads;
	FD_ZERO(&reads);
	FD_SET(server, &reads);
	SOCKET max_socket = server;

	std::list<ClientInfo>::iterator it = clients.begin();
	std::list<ClientInfo>::iterator end = clients.end();
	for (; it != end; it++) {
		FD_SET(it->socket, &reads);
		max_socket = it->socket > max_socket ? it->socket : max_socket;
	}

	if (select(max_socket+1, &reads, NULL, NULL, NULL) < 0) {
		std::cerr << "select() failed. (" << errno << ")\n";
		exit(1);
	}
	return reads;
}

std::list<ClientInfo>::iterator	send400(ClientInfo& client, std::list<ClientInfo>& clients) {
	const std::string c400 = "HTTP/1.1 400 Bad Request\r\n"
							 "Connection: close\r\n"
							 "Content-Length: 11\r\n\r\nBad Request";
	send(client.socket, c400.c_str(), c400.length(), 0);
	return dropClient(client, clients);
}

std::list<ClientInfo>::iterator	send404(ClientInfo& client, std::list<ClientInfo>& clients) {
	const std::string c404 = "HTTP/1.1 404 Not Found\r\n"
							 "Connection: close\r\n"
							 "Content-Length: 9\r\n\r\nNot Found";
	send(client.socket, c404.c_str(), c404.length(), 0);
	return dropClient(client, clients);
}

std::list<ClientInfo>::iterator	serveResource(ClientInfo& client, std::string path,
						std::list<ClientInfo>& clients) {
	std::cout << "serveRessource() " << getClientAddress(client) << ' ' << path << std::endl;

	if (path == "/") path = "/index.html";

	if (path.length() > 100){
		return send400(client, clients);
	}

	if (path.find("..") != std::string::npos) {
		return send404(client, clients);
	}
	std::string full_path = "public" + path;



	// std::fstream resource(full_path.c_str(), std::ios::binary | std::ios::ate);
	std::fstream resource(full_path.c_str(), std::ios::binary | std::ios::ate | std::ios::in);
	std::cout << full_path << std::endl;
	if (!resource.is_open()) {
		return send404(client, clients);
	}

	size_t content_length = static_cast<size_t>(resource.tellg());
	resource.seekg(0, std::ios::beg);

	std::string content_type = getContentType(full_path);

	std::stringstream headers_buffer;
	headers_buffer << "HTTP/1.1 200 OK\r\n"
		   << "Connection: close\r\n"
		   << "Content-Length: " << content_length << "\r\n"
		   << "Content-type: " << content_type << "\r\n"
		   << "\r\n";

	std::string headers = headers_buffer.str();
	send(client.socket, headers.c_str(), headers.length(), 0);

	char sending_buffer[1024];
	resource.read(sending_buffer, 1024);
	while (resource.gcount()) {
		send(client.socket, sending_buffer, resource.gcount(), 0);
		resource.read(sending_buffer, 1024);
	}

	resource.close();
	return dropClient(client, clients);
}

int main() {
	SOCKET server = createSocket("", "3000");
	std::list<ClientInfo> clients;

	while (true) {
		fd_set reads;
		reads = waitOnClients(server, clients);

		if (FD_ISSET(server, &reads)) {
			ClientInfo& client = getClient(-1, clients);

			client.socket = accept(server, (struct sockaddr*)&client.address,
								  &client.address_length);

			if (!IsValidSocket(client.socket)) {
				std::cerr << "accept() failed. (" << errno << ")\n";
				return 1;
			}

			std::cout << "New connection from " << getClientAddress(client) << ".\n";
		}

		std::list<ClientInfo>::iterator client = clients.begin();
		while (client != clients.end()) {
			//INFO: if the client is not ready pass to the next one
			if (!FD_ISSET(client->socket, &reads)) {
				client++;
				continue;
			}


			if (MAX_REQUEST_SIZE == client->received) {
				client = send400(*client, clients);
				continue;
			}

			int received_bytes = recv(client->socket,
							 client->request + client->received,
							 MAX_REQUEST_SIZE - client->received, 0);

			if (received_bytes < 1) {
				std::cout << "Unexpected disconnect from "
					<< getClientAddress(*client) << ".\n";
				client = dropClient(*client, clients);
			} else {

				client->received += received_bytes;
				client->request[client->received] = 0;

				// INFO: start processing the request when all headers are received
				char* headers_received = strstr(client->request, "\r\n\r\n");
				if (headers_received) {
					if (!strncmp("GET /", client->request, 5)) {
						char *path = client->request + 4;
						char *end_path = strstr(path, " ");
						if (!end_path) {
							client = send400(*client, clients);
						} else {
							*end_path = 0;
							client = serveResource(*client, path, clients);
						}
					} else {
						client = send400(*client, clients);
					}
				} // if q
			}
		}
	} // while true

	std::cout << "\nClosing socket...\n";
	CloseSocket(server);

	std::cout << "Finished." << std::endl;
	return 0;
}
