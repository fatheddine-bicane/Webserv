#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

enum TokenType{
	TOKEN_SERVER,       // "server"
	TOKEN_LOCATION,     // "location"
	TOKEN_LISTEN,       // "listen"
	TOKEN_SERVER_NAME,  // "server_name"
	TOKEN_ROOT,         // "root"
	TOKEN_INDEX,        // "index"
	TOKEN_AUTOINDEX,    // "autoindex"
	TOKEN_ERROR_PAGE,   // "error_page"
	TOKEN_MAX_BODY_SIZE, // "client_max_body_size"
	TOKEN_LIMIT_EXCEPT, // "limit_except"
	TOKEN_ALIAS,        // "alias"
	TOKEN_RETURN,       // "return"
	TOKEN_DAV_METHODS,   // "dav_methods"
	TOKEN_CLIENT_BODY_TEMP_PATH, // "client_body_temp_path"
	TOKEN_CREATE_FULL_PUT_PATH, // "create_full_put_path"
	TOKEN_LBRACE,       // "{"
	TOKEN_RBRACE,       // "}"
	TOKEN_SEMICOLON,    // ";"
	TOKEN_IDENTIFIER,   // Any unknown word (arguments, paths, ports, e.g., "8080", "/tmp")
	TOKEN_EOF           // end of file
};

struct Token{
	TokenType type;
	std::string value;
};

class Lexer{
private:
	std::string _filePath;
	std::map<std::string, TokenType> _keywords;
	void _initKeywordMap();
	Lexer(const Lexer &src);
	Lexer& operator=(const Lexer &src);
public:
	Lexer(const std::string& filePath);
	~Lexer();
	std::vector<Token> tokenize();
};
