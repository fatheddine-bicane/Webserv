#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "./config_parser/Scanner/Definitions/Scanner.hpp"
#include "config_parser/Parser/Definitions/Parser.hpp"
#include "config_parser/Scanner/Definitions/Token.hpp"
#include "Server_socket_multiplexing/Definitions/EpollMultiplexer.hpp"

int main(int argc, char** argv) {
	Servers* servers = NULL;
	EpollMultiplexer* multiplexer = NULL;

	try {
		// initialise scanner
		Scanner scanner(argc, argv);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, *scanner.source);
		parser.scanTokens();
		servers = new Servers(parser.getServers());
		
		std::cout << "[MAIN] Config parsed successfully" << std::endl;
		std::cout << "[MAIN] Found " << parser.getAddresses().size() << " listen directive(s)" << std::endl;

		// Initialize EpollMultiplexer
		multiplexer = new EpollMultiplexer(&parser.getAddresses());

		if (!multiplexer->init()) {
			std::cerr << "[MAIN] Failed to initialize EpollMultiplexer" << std::endl;
			return 1;
		}
		
		std::cout << "[MAIN] EpollMultiplexer initialized successfully" << std::endl;

		multiplexer->eventLoop();
		(void)servers;
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 2;
	}
	if (multiplexer != NULL) {
		delete multiplexer;
	}
	if (servers != NULL) {
		delete servers;
	}
	return 0;
}
