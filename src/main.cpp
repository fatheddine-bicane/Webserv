#include "./config_file/lexer/includes/Lexer.hpp"


std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_SERVER:                  return "SERVER";
        case TOKEN_LOCATION:                return "LOCATION";
        case TOKEN_LISTEN:                  return "LISTEN";
        case TOKEN_SERVER_NAME:             return "SERVER_NAME";
        case TOKEN_ROOT:                    return "ROOT";
        case TOKEN_INDEX:                   return "INDEX";
        case TOKEN_AUTOINDEX:               return "AUTOINDEX";
        case TOKEN_ERROR_PAGE:              return "ERROR_PAGE";
        case TOKEN_MAX_BODY_SIZE:           return "MAX_BODY_SIZE";
        case TOKEN_LIMIT_EXCEPT:            return "LIMIT_EXCEPT";
        case TOKEN_ALIAS:                   return "ALIAS";
        case TOKEN_RETURN:                  return "RETURN";
        case TOKEN_DAV_METHODS:             return "DAV_METHODS";
        case TOKEN_CLIENT_BODY_TEMP_PATH:   return "CLIENT_BODY_TEMP_PATH";
        case TOKEN_CREATE_FULL_PUT_PATH:    return "CREATE_FULL_PUT_PATH";
        case TOKEN_LBRACE:                  return "LBRACE ({)";
        case TOKEN_RBRACE:                  return "RBRACE (})";
        case TOKEN_SEMICOLON:               return "SEMICOLON (;)";
        case TOKEN_IDENTIFIER:              return "IDENTIFIER";
        case TOKEN_EOF:                     return "EOF";
        default:                            return "UNKNOWN_TOKEN";
    }
}

int main(int ac, char **av) {
	if (ac != 2) {
		std::cerr << "Usage: ./webserv [configuration_file]" << std::endl;
		return 1;
	}

	try {
		Lexer MyLexer(av[1]);
		std::vector<Token> tokens = MyLexer.tokenize();
		std::cout << "========================================================" << std::endl;
		std::cout << "                LEXER TOKENIZATION OUTPUT               " << std::endl;
		std::cout << "========================================================" << std::endl;
		std::cout << "Total Tokens Found: " << tokens.size() << std::endl << std::endl;

		for (size_t i = 0; i < tokens.size(); ++i) {
			std::cout << "Token [" << i << "]:" << std::endl;
			std::cout << "  Type:  " << tokenTypeToString(tokens[i].type) << std::endl;
			std::cout << "  Value: \"" << tokens[i].value << "\"" << std::endl;
			std::cout << "--------------------------------------------------------" << std::endl;
		}

	} catch (const std::exception& e) {
		std::cerr << "Fatal Error during lexing: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
