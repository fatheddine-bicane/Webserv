#include "Includes/Webserv.hpp"


int main(int argc, char** argv) {
	Webserv	webserv;

	try {
		// initialise scanner
		Scanner scanner(argc, argv);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, *scanner.source);
		parser.scanTokens();
		webserv.setServers(parser.getServers());

		// Initialize EpollMultiplexer
		ServerMultiplexing multiplexer = ServerMultiplexing(parser.getAddresses(), webserv.epfd);
		multiplexer.bootstrapServerListeners();
		webserv.setSocketsMap(multiplexer.getSocketsMap());
	} catch (ParserException& e) {
		std::cout << e.what() << std::endl;
		return 2;
	} catch (SystemCallsFailedException& e) {
		std::cout << e.what() << std::endl;
		return 2;
	}

	return 0;
}
