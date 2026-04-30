#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <cstdio>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <ctype.h>


typedef int SOCKET;
#define IsValidSocket(s) ((s) >= 0)
#define CloseSocket(s) (close(s))

int main() {
	std::cout << "Configuring local address...\n";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo* bind_address;
	int status = getaddrinfo(NULL, "8080", &hints, &bind_address);

	std::cout << "Creating socket...\n";
	SOCKET socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (!IsValidSocket(socket_listen)) {
		std::cerr << "socket() failed. (" << errno << ")\n";
		return 1;
	}

	std::cout << "Binding socket...\n";
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
		std::cerr << "bind() failed. (" << errno << ")\n";
		return 1;
	}
	freeaddrinfo(bind_address);

	std::cout << "Listening...\n";
	if (listen(socket_listen, 10) < 0) {
		std::cerr << "listen()  failed. (" << errno << ")\n";
		return 1;
	}

	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	SOCKET max_socket = socket_listen;

	std::cout << "Waiting for connections...\n";
	while (true) {
		fd_set reads = master;

		if (select(max_socket+1, &reads, NULL, NULL, NULL) < 0) {
			std::cerr << "select() failed. (" << errno << ")\n";
			return 1;
		}

		for (SOCKET i = 1; i <= max_socket; i++) {
			if (!FD_ISSET(i, &reads)) {
				continue;
			}
			if (i == socket_listen) {
				struct sockaddr_storage client_address;
				socklen_t client_len = sizeof(client_address);
				SOCKET socket_client = accept(socket_listen, (struct sockaddr*)&client_address, &client_len);
				if (!IsValidSocket(socket_client)) {
					std::cerr << "accept() failed. (" << errno << ")\n";
					return 1;
				}

				FD_SET(socket_client, &master);
				if (socket_client > max_socket) {
					max_socket = socket_client;
				}

				char address_buffer[100];
				getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, 100, NULL, 0, NI_NUMERICHOST);
				std::cout << "New connection from: " << address_buffer << std::endl;
			} else {
				char read[1024];
				int bytes_received = recv(i, read, 1024, 0);
				if (bytes_received < 1) {
					FD_CLR(i, &master);
					CloseSocket(i);
					continue;
				}

				for (int j = 0; j < bytes_received; j++) {
					read[j] = toupper(read[j]);
				}
				send(i, read, bytes_received, 0);

				// INFO:chat room example
				// for (SOCKET j = 1; j <= max_socket; j++) {
				// 	// for each socket connected to our server
				// 	if (FD_ISSET(j, &master)) {
				// 		// if this socket is the sender nor the listening socket, send the message
				// 		if (j == socket_listen || j == i) {
				// 			continue;
				// 		} else {
				// 			send(j, read, bytes_received, 0);
				// 		}
				// 	}
				// }

				/*
				INFO: this code ensures to send all data ( if data is too large to fit in the
				kenel given outgoing buffer, wee then need to trace how many bytes sent)
				int begin = 0;
				char* buffer;
				int buffer_len = 0; // content of buffer
				SOCKET socket_client;
				while (begin < buffer_len) {
					int bytes_sent = send(socket_client, buffer + begin, buffer_len - begin, 0);
					if (bytes_sent == -1) {
						// handle error
					}
					begin += bytes_sent;
				}
				WARNING: this method is blocking blocking!
				WARNING: consider buffering data befor processing it, a server might
				not receive all content for correct processing--server expect 'quit'
				to stop runing, recv() returned 'qui'-- because TCP is a streaming
				protocol, therefor, we have to buffer data from recv() until a
				suitable amount is available to interpret. For TCP peers that are
				handling large amounts of data, buffering to send() is also necessary.
				*/


			}
		} // for i to max_socket
	} // while true

	std::cout << "Closing listening socket...\n";
	CloseSocket(socket_listen);

	std::cout << "Fineshed.\n";
}
