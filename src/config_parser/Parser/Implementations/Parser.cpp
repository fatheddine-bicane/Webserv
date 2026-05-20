#include "../Definitions/Parser.hpp"


// INFO: constructors
// ---------------------------------------------------------

Parser::Parser(std::vector<Token> tokens, String& source) 
	: _source(source) {
	this->_current = 0;
	this->_tokens = tokens;

	// append to namless servers
	this->_servers_count = 0;
}

// ---------------------------------------------------------



// INFO: API
// ---------------------------------------------------------

Servers	Parser::scanTokens() {
	SharedDirectives	http_context;
	Servers				servers;

	while (!isAtEnd()) {
		scanToken(servers, http_context);
	}

	return servers;
}

// ---------------------------------------------------------



// INFO: utility functions
// -----------------------------------------------------------------

void	Parser::scanToken(Servers& servers, SharedDirectives& http_context) {
	Token token = consume();

	switch (token._token_type) {
		default: break;
	}
}


Token	Parser::consume() {
	return (this->_tokens.at(this->_current++));
}


Token	Parser::currentToken() {
	return (this->_tokens.at(this->_current - 1));
}


Token	Parser::peek() {
	return (this->_tokens.at(this->_current));
}


void	Parser::expect(TokenType token_type) {
	if (consume()._token_type != token_type) {
		throw ExpectedTokenException(currentToken(), this->_source);
	}
}


bool	Parser::match(TokenType to_match) {
	return (this->_tokens.at(this->_current - 1)._token_type == to_match);
}


bool	Parser::match(Token token, TokenType to_match) {
	return (token._token_type == to_match);
}


bool	Parser::isAtEnd() {
	return (peek()._token_type == END_OF_FILE);
}

// -----------------------------------------------------------------

	Token token = consume();
