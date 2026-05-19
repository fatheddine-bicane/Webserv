#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "./config_parser/Scanner/Definitions/Scanner.hpp"
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


	// initialise scanner 
	String source = ss.str();
	Scanner scanner(source);

	// scan config file
	try {
		std::vector<Token> tokens = scanner.scanTokens();


		std::vector<Token>::iterator it = tokens.begin();
		std::vector<Token>::iterator end = tokens.end();
		for (; it != end; it++) {
			std::cout << it->toString(source);
		}

	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 2;
	}
}
