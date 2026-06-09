#include "Includes/Webserv.hpp"
#include "Server_socket_multiplexing/Definitions/ServerMultiplexing.hpp"


int main(int argc, char** argv) {
	Servers* servers = NULL;
	SocketsMap* sockets_map = NULL;
	EP_INSTANCE epfd = epoll_create(1);

	try {
		// initialise scanner
		Scanner scanner(argc, argv);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, *scanner.source);
		parser.scanTokens();
		servers = new Servers(parser.getServers());
		

		// Initialize EpollMultiplexer
		ServerMultiplexing multiplexer = ServerMultiplexing(parser.getAddresses(), epfd);
		multiplexer.bootstrapServerListeners();
		sockets_map = new SocketsMap(multiplexer.getSocketsMap());

		while (true) {
		struct epoll_event events[10];
		int n = epoll_wait(epfd, events, 10, 10);

		for (int i = 0; i < n; i++) {

			Connection* connection = static_cast<Connection*>(events[i].data.ptr);

			// if its a listening socket
			if (connection->type == SERVER_S) {
				String ip = sockets_map->at(connection->fd);
				std::cout << "new connection to: " << ip << std::endl;

				SOCKET socket_client = accept(connection->fd, NULL, NULL);
				ClientConnection* client_connection = new ClientConnection(socket_client);

				struct epoll_event event;
				event.events = EPOLLIN;
				event.data.ptr = client_connection;
				int status = epoll_ctl(epfd, EPOLL_CTL_ADD, socket_client, &event);
				if (status == -1) {
					delete client_connection;
				}
				std::cout << "client added" << std::endl;
			}


			// if its a client socket
			else if (connection->type == CLIENT_S){
				std::cout << "new packet arrived" << std::endl;

				char buffer[1000];
				recv(connection->fd, buffer, 1000, 0);
				std::cout << buffer << std::endl;
			}
		}
		(void)servers;
		}
	} catch (std::exception& e) {

		std::cout << e.what() << std::endl;
		return 2;
	}
	return 0;
}
