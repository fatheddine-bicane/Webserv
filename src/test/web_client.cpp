#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <cstdlib>
#include <ctime>
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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <iostream>

typedef int SOCKET;
#define IsValidSocket(s) ((s) >= 0)
#define CloseSocket(s) (close(s))
#define TIMEOUT 5.0

void	parseUrl(std::string& url, std::string& hostname, std::string& port, std::string& path) {
	std::cout << "URL: " << url << std::endl;

	std::string::iterator p = url.begin();

	// protocol
	size_t protocol_pos = url.find("://");
	if (protocol_pos != std::string::npos) {
		std::string protocol = url.substr(0, protocol_pos);
		if (protocol != "http") {
			std::cerr << "Unknown protocol '" << protocol << "'. Only 'http' is supported.\n";
			exit(1);
		}
		p += protocol_pos + 3;
	}

	// hostname
	while (*p && *p != ':' && *p != '/' && *p != '#') {
		hostname.push_back(*p);
		p++;
	}

	// port
	if (*p == ':') {
		p++;
		while (*p && *p != '/' && *p != '#') {
			port.push_back(*p);
			p++;
		}
	} else {
		port = "80";
	}

	// path
	while (*p && *p != '#') {
		path.push_back(*p);
		p++;
	}

	std::cout << "Hostname: " << hostname << std::endl;
	std::cout << "Port: " <<  port << std::endl;
	std::cout << "Path: " << path << std::endl;
}

void	sendRequest(SOCKET s, std::string& hostname, std::string& port, std::string& path) {
	std::stringstream buffer;

	buffer << "GET " << path << " HTPP/1.1\r\n";
	buffer << "Host: " << hostname << ':' << port << "\r\n";
	buffer << "Connection: close\r\n";
	buffer << "User-Agent: web_client 1.0\r\n";
	buffer << "\r\n";

	std::string request_headers = buffer.str();
	send(s, request_headers.c_str(), request_headers.length(), 0);

	std::cout << "\nSent headers: " << request_headers;
}

SOCKET	connectToHost(const std::string& hostname, const std::string& port) {
	std::cout << "Configuring remote address...\n";

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* peer_address;
	int status;
	status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &peer_address);
	if (status) {
		std::cerr << "getaddrinfo() failed. (" << errno << ")\n";
		exit(1);
	}

	std::cout << "Remote address is: ";
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
			 address_buffer, 100,
			 service_buffer, 100, NI_NUMERICHOST);
	std::cout << address_buffer << ' ' << service_buffer << std::endl;

	std::cout << "Creating socket...\n";
	SOCKET server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if (!IsValidSocket(server)) {
		std::cerr << "socket() failed. (" << errno << ")\n";
		exit(1);
	}
	std::cout << "Connecting...\n";
	status = connect(server, peer_address->ai_addr, peer_address->ai_addrlen);
	if (status) {
		std::cerr << "connect() failed. (" << errno << ")\n";
		exit(1);
	}
	freeaddrinfo(peer_address);

	std::cout << "Connected.\n";

	return server;
}

#define RESPONSE_SIZE 8192

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: ./program url\n";
		return 1;
	}

	std::string url = argv[1];

	std::string hostname, port, path;
	parseUrl(url, hostname, port, path);
	SOCKET server = connectToHost(hostname, port);
	sendRequest(server, hostname, port, path);

	const clock_t start_time = clock();

	char responce[RESPONSE_SIZE + 1]; // responce buffer
	char* p = responce; // keep track of how fare we have witten into responce so far
	char* q;
	char* end = responce + RESPONSE_SIZE; // end of the buffer prevent exceding to responce buffer
	char* body = NULL; // remember the beginning of the received HTTP body responce

	enum {length, chunked, connection};
	int encoding = length;
	int remaining = 0;

	// receive and process HTTP request
	while (true) {
		// check for timeout
		if ((clock() - start_time) / CLOCKS_PER_SEC	> TIMEOUT) {
			std::cerr << "Timeout after " << TIMEOUT << ".2f seconds\n";
			return 1;
		}

		// check buffer space
		if (p == end) {
			std::cerr << "Out of buffer space\n";
			return 1;
		}

		// receive data from socket
		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(server, &reads);

		struct timeval timeout;
		timeout.tv_usec = 200000;

		timeout.tv_sec = 0;
		if (select(server + 1, &reads, NULL, NULL, &timeout) < 0) {
			std::cerr << "select() failed. (" << errno << ")\n";
			return 1;
		}

		if (FD_ISSET(server, &reads)) {
			int bytes_received = recv(server, p, end - p, 0);
			if (bytes_received <= 0) {
				if (encoding == connection && body) {
					std::cout << body;
				}
				std::cout << "\nConnection closed by peer.\n";
				break;
			}
			// std::cout << "Received (" << bytes_received << " bytes): " << p;
			p += bytes_received;
			*p = '\0';

			if (!body && (body = strstr(responce, "\r\n\r\n"))) {
				*body = '\0';
				body += 4;

				std::cout << "Received headers:\n" << responce << std::endl;
			}

			// indicate the body encoding
			q = strstr(responce, "\nContent-Length: ");
			if (q) {
				encoding = length;
				q = strchr(q, ' ') + 1;
				remaining = strtol(q, NULL, 10);
			} else {
				q = strstr(responce, "\nTransfer-Encoding: chunked");
				if (q) {
					encoding = chunked;
					remaining = 0;
				} else {
					encoding = connection;
				}
			}
			std::cout << "\nReceived body:\n";

			if (body) {
				if (encoding == length) {
					if (p - body >= remaining) {
						std::cout << body;
						break;
					}
				} else if (encoding == chunked) {
					//remaining is used to indicate whether a chunck length or chunck data is expected next
					do {
						if (remaining == 0) {
							if ((q = strstr(body, "\r\n"))) {
								remaining = strtol(body, NULL, 16);
								if (remaining == 0) goto finish;
								body = q + 2;
							} else {
								break;
							}
						}
						if (remaining != 0 && p - body >= remaining) {
							std::cout << body;
							body += remaining + 2;
							remaining = 0;
						}
					} while (remaining == 0);
				}
			} // if body

		}// if FD_ISSET
	} // while true

finish:
	std::cout << "\nClosing socket...\n";
	CloseSocket(server);

	std::cout << "Finished.\n";
	return 0;
}
