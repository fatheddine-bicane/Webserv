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
	static bool is_events_parsed = false;
	static bool is_http_parsed = false;

	Token token = consume();

	switch (token._token_type) {
		case EVENTS:
			if (is_events_parsed) {
				throw DuplicatedDirectiveException(currentToken(), this->_source);
			}
			parseEvents();
			is_events_parsed = true;
			break;
		case HTTP:
			if (is_http_parsed) {
				throw DuplicatedDirectiveException(currentToken(), this->_source);
			}
			parseHttp(servers, http_context);
			is_http_parsed = true;
			break;


		default: break;
	}
}


Token	Parser::consume() {
	return (this->_tokens.at(this->_current++));
}


Token	Parser::currentToken() {
	return (this->_tokens.at(this->_current - 1));
}


Token	Parser::previousToken() {
	return (this->_tokens.at(this->_current - 2));
}


Token	Parser::peek() {
	return (this->_tokens.at(this->_current));
}


void	Parser::expect(TokenType token_type) {
	if (consume()._token_type != token_type) {
		String missing_token;
		switch (token_type) {
			case CONTEXT_START: missing_token = "{"; break;
			case CONTEXT_END: missing_token = "}"; break;
			case SEMICOLON: missing_token = ";"; break;

			default: break;
		}
		throw ExpectedTokenException(previousToken(), missing_token, this->_source);
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



// INFO: block directive parsers (context)
// -----------------------------------------------------------------

void	Parser::parseEvents() {
	expect(CONTEXT_START);
	expect(CONTEXT_END);
}

void	Parser::parseHttp(Servers& servers, SharedDirectives& http_context) {
	(void) servers;
	expect(CONTEXT_START);
	Token token = consume();

	while (!match(token, CONTEXT_END)) {
		switch (token._token_type) {
			case ROOT: parseRoot(http_context); break;

			default: break;
		}
		token = consume();
	}
}

// -----------------------------------------------------------------



// INFO: simple directive parsers
// -----------------------------------------------------------------

void	Parser::parseRoot(SharedDirectives& http_context) {
	Token token = consume();
	http_context.root = token._lexeme;
	expect(SEMICOLON);
}

// -----------------------------------------------------------------
