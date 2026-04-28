#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include <ctime>



// might not be needed
typedef int SOCKET;
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) (close(s))

int	main() {
	std::cout << "Configuring local address ...\n";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* bind_address;
	int	status = getaddrinfo(NULL, "8080", &hints, &bind_address);

	std::cout << "Creating socket ...\n";
	
	SOCKET	socket_listen;
	socket_listen = socket(bind_address->ai_family,
						   bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen)) {
		std::cerr << "socket() failed. (" << errno << ")\n";
		return 1;
	}

	// listen on both IPv4 and IPv6 (if the system supports dual-stack socket).
	// the socket should be initially created as an IPv6 socket (hints.ai_family = AF_INET6)
	int option = 0;
	int result = setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option));
	if (result) {
		std::cerr << "setsockopt() failed. (" << errno << ")\n";
		return 1;
	}

	std::cout << "Binding socket to a local address...\n";
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
		std::cerr << "bind() failed. (" << errno << ")\n";
		return 1;
	}
	freeaddrinfo(bind_address);

	// start listening
	std::cout << "Listening ...\n";
	if (listen(socket_listen, 10)) {
		std::cerr << "listen() failed. (" << errno << ")\n";
		return 1;
	}

	// establishing a new connections
	std::cout << "Waiting for connection ...\n";
	struct sockaddr_storage client_address;
	socklen_t	client_len = sizeof(client_address);
	SOCKET socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
	if (!ISVALIDSOCKET(socket_client)) {
		std::cerr << "accept() failed. (" << errno << ")\n";
		return 1;
	}

	// printing client address
	std::cout << "Client is connected ...\n";
	char address_buffer[100];
	getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	std::cout << address_buffer << '\n';

	// reading request
	std::cout << "Reading request ...\n";
	char request[1024];
	// WARNING: always check recv return value (0 or -1)
	int bytes_received = recv(socket_client, request, 1024, 0);
	std::cout << "Received " << bytes_received << " bytes.\n";
	// printing the request, WARNING: migt cause a segfault, request is not guarented to be null terminated.
	std::cout << "Request: " << request << std::endl;

	std::cout << "Sending responce ...\n";
	const char*	response =
		"HTTP/1.1 200 OK\r\n"
		"Conection: close\r\n"
		"Content-type: text/plain\r\n\r\n"
		"Local time is: ";
	int bytes_sent = send(socket_client, response, strlen(response), 0);
	time_t timer;
	time(&timer);
	char* time_msg = ctime(&timer);
	bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
	std::cout << "Sent " << bytes_sent << " of " << strlen(time_msg) << "\n";

	std::cout << "Closing conecction ...\n";
	CLOSESOCKET(socket_client);

	std::cout << "Closing listening socket ...\n";
	CLOSESOCKET(socket_listen);
	std::cout << "Finished.\n";
	return 0;

}
