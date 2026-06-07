#include "Includes/webserv.hpp"
#include "Server_socket_multiplexing/Definitions/ServerSocketsMultiplexing.hpp"


int main(int argc, char** argv) {
	Servers* servers = NULL;
	SocketsMap* sockets_map = NULL;
	int epfd = epoll_create(1);

	try {
		// initialise scanner
		Scanner scanner(argc, argv);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, *scanner.source);
		parser.scanTokens();
		servers = new Servers(parser.getServers());


		// initialize socket multiplexing
		ServerSocketsMultiplexing ss_multiplexing(parser.getAddresses(), epfd);
		ss_multiplexing.setupServerSockets();
		sockets_map = new SocketsMap(ss_multiplexing.getSocketsMap());




		// test sockets
		while (true) {
			struct epoll_event events[10];
			int n = epoll_wait(epfd, events, 10, 10);

			for (int i = 0; i < n; i++) {
				String ip = sockets_map->at(events[i].data.fd);
				std::cout << "new connection to: " << ip << std::endl;
			}
		}


	} catch (ParserException& e) {
		std::cerr << e.what() << std::endl;
		close(epfd);
		return 2;
	} catch (SystemCallsFailedException& e) {
		std::cerr << e.what() << std::endl;
		close(epfd);
		delete servers;
	}
}
