#include <bits/types/struct_timeval.h>
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


typedef int SOCKET;
#define IsValidSocket(s) ((s) >= 0)
#define CloseSocket(s) (close(s))


int	main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: ./program [hostname] [port]\n";
		return 1;
	}

	std::cout << "Configuring remote address ...\n";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* peer_address = NULL;
	if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
		std::cerr << "getaddinfo() failed. (" << errno << ")\n";
		return 1;
	}

	std::cout << "Remote address is: ";
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
			 address_buffer, sizeof(address_buffer),
			 service_buffer, sizeof(service_buffer),
			 NI_NUMERICHOST);
	std::cout << address_buffer << ' ' << service_buffer << '\n';

	std::cout << "Creating socket ...\n";
	SOCKET peer_socket = socket(peer_address->ai_family,
							    peer_address->ai_socktype, peer_address->ai_protocol);
	if (!IsValidSocket(peer_socket)) {
		std::cerr << "socket() failed. (" << errno << ")\n";
	}

	std::cout << "Connecting ...\n";
	if (connect(peer_socket, peer_address->ai_addr, peer_address->ai_addrlen)) {
		std::cerr << "connect() failed. (" << errno << ")\n";
	}
	freeaddrinfo(peer_address);

	std::cout << "Connected.\n";
	std::cout << "To send data, enter text folowed by enter.\n";

	while (true) {
		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(peer_socket, &reads);
		FD_SET(STDIN_FILENO, &reads);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		if (select(peer_socket + 1, &reads, 0, 0, &timeout) < 0) {
			std::cerr << "select() failed. (" << errno << ")\n";
			return 1;
		}

		if (FD_ISSET(peer_socket, &reads)) {
			char read[4096];
			int bytes_received = recv(peer_socket, read, 4096, 0);
			if (bytes_received < 1) {
				std::cerr << "Connection closed by peer.\n";
				break;
			}
			std::cout << "Received (" << bytes_received << "): " << read << '\n';

		}

		if (FD_ISSET(STDIN_FILENO, &reads)) {
			char read[4096];
			if (!fgets(read, 4096, stdin))
				break;
			std::cout << "Sending ...\n";
			int bytes_sent = send(peer_socket, read, strlen(read), 0);
			std::cout << "Sent " << bytes_sent << " bytes.\n";
		}
	}
	std::cout << "Closing socket ...\n";
	CloseSocket(peer_socket);
	std::cout << "Fineshed.\n";
	return 0;
}
