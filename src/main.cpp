#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "./config_parser/Scanner/Definitions/Scanner.hpp"
#include "config_parser/Parser/Definitions/Parser.hpp"
#include "config_parser/Scanner/Definitions/Token.hpp"

int main(int argc, char** argv) {
	std::cout << "Hello from webserv" << std::endl;
	(void) argc;
	(void) argv;

	// read config file content
	std::fstream f(argv[1], std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		std::cerr << "file not found";
		return 1;
	}
	std::stringstream ss;
	ss << f.rdbuf();
	std::cout << "------------------" << std::endl;


	String source = ss.str();

	// scan config file
	try {
		// initialise scanner 
		Scanner scanner(source);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, source);
		parser.scanTokens();



		// std::vector<Token>::iterator it = tokens.begin();
		// std::vector<Token>::iterator end = tokens.end();
		// for (; it != end; it++) {
		// 	std::cout << it->toString(source);
		// }

	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 2;
	}
}
