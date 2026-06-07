#include "Includes/webserv.hpp"


int main(int argc, char** argv) {
	Servers* servers = NULL;

	try {
		// initialise scanner
		Scanner scanner(argc, argv);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, *scanner.source);
		parser.scanTokens();
		servers = new Servers(parser.getServers());
		(void)servers;


	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 2;
	}
}
