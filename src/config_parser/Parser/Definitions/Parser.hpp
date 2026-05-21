#pragma once

#include <vector>
#include <utility>
#include <map>
#include <cstdlib>

#include "../Definitions/Directives.hpp"
#include "../../Scanner/Definitions/Token.hpp"
#include "../../Scanner/Definitions/TokenType.hpp"
#include "../Exceptions/ParserException.hpp"

typedef std::string String;
typedef std::map<String, Server>	Servers;


class Parser {
private:
	String&				_source;
	int					_current;
	std::vector<Token>	_tokens;
	int					_servers_count;


// INFO: constructors
public:
	Parser(std::vector<Token> tokens, String& source);


// INFO: API
public:
	Servers	scanTokens();


// INFO: utility functions
private:
	void	scanToken(Servers& servers, SharedDirectives& directive_context);

	Token	consume();
	Token	currentToken();
	Token	previousToken();
	Token	peek();
	void	expect(TokenType token_type);
	bool	match(TokenType to_match);
	bool	match(Token token, TokenType to_match);
	bool	isAtEnd();


// INFO: block directive parsers (context)
private:
	void	parseEvents();
	void	parseHttp(Servers& servers, SharedDirectives& directive_context);


// INFO: simple directive parsers
private:
	void	parseRoot(SharedDirectives& directive_context);
	void	parseErrorPage(SharedDirectives& directive_context);

};
