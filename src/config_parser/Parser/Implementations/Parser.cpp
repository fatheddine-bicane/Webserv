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
	SharedDirectives	directive_context;
	Servers				servers;

	while (!isAtEnd()) {
		scanToken(servers, directive_context);
	}

	return servers;
}

// ---------------------------------------------------------



// INFO: utility functions
// -----------------------------------------------------------------

void	Parser::scanToken(Servers& servers, SharedDirectives& directive_context) {
	static bool is_events_parsed = false;
	static bool is_http_parsed = false;

	Token token = consume();

	switch (token.type) {
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
			parseHttp(servers, directive_context);
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
	if (consume().type != token_type) {
		String missing_token;
		switch (token_type) {
			case CONTEXT_START: missing_token = "{"; break;
			case CONTEXT_END: missing_token = "}"; break;
			case SEMICOLON: missing_token = ";"; break;

			default: break;
		}
		Token previous_token = previousToken();
		previous_token.spaces++;
		throw ExpectedTokenException(previous_token, missing_token, this->_source);
	}
}


bool	Parser::match(TokenType to_match) {
	return (this->_tokens.at(this->_current - 1).type == to_match);
}


bool	Parser::match(Token token, TokenType to_match) {
	return (token.type == to_match);
}


bool	Parser::isAtEnd() {
	return (peek().type == END_OF_FILE);
}

bool	Parser::isDirective(TokenType token_type) {
	switch (token_type) {
		case EVENTS:
		case HTTP:
		case SERVER:
		case LOCATION:
		case LIMIT_EXCEPT:
		case LISTEN:
		case ERROR_PAGE:
		case ROOT:
		case CLIENT_MAX_BODY_SIZE:
		case AUTOINDEX:
		case INDEX:
		case ALIAS:
		case RETURN:
		case DAV_METHODS:
		case CLIENT_BODY_TEMP_PATH:
		case CREATE_FULL_PUT_PATH:
		case CGI_PASS:
			return true;

		default:
			return false;
	}
}

// -----------------------------------------------------------------



// INFO: block directive parsers (context)
// -----------------------------------------------------------------

void	Parser::parseEvents() {
	expect(CONTEXT_START);
	expect(CONTEXT_END);
}


void	Parser::parseHttp(Servers& servers, SharedDirectives& directive_context) {
	(void) servers;
	expect(CONTEXT_START);
	Token token = consume();

	while (!match(token, CONTEXT_END)) {
		switch (token.type) {
			case ROOT: parseRoot(directive_context); break;
			case ERROR_PAGE: parseErrorPage(directive_context); break;
			case CLIENT_MAX_BODY_SIZE:
				parseClientMaxBodySize(directive_context);
				break;
			case CLIENT_BODY_TEMP_PATH:
				parseClientBodyTempPath(directive_context);
				break;
			case AUTOINDEX: parseAutoindex(directive_context); break;
			case INDEX: parseIndex(directive_context); break;
			case DAV_METHODS: parseDavMethods(directive_context); break;
			case CREATE_FULL_PUT_PATH:
				parserCreateFullPutPath(directive_context);
				break;

			default:
				if (isDirective(token.type)) {
					throw DirectiveNotAllowedHereException(token, this->_source);
				} else {
					throw UnexpectedTokenException(token, "", this->_source);
				}
		}
		token = consume();
	}
}

// -----------------------------------------------------------------



// INFO: simple directive parsers
// -----------------------------------------------------------------

void	Parser::parseRoot(SharedDirectives& directive_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	directive_context.root = token.lexeme;
	expect(SEMICOLON);
}


void	Parser::parseErrorPage(SharedDirectives& directive_context) {
	Token error_page = currentToken();
	std::vector<int> error_codes;
	int argument_count = 0;

	Token token = consume();

	// no value provided
	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	while (!match(peek(), SEMICOLON) && match(peek(), VALUE)) {
		char*	end = NULL;
		long	error_code = std::strtol(token.lexeme.c_str(), &end, 10);
		if (*end != '\0') {
			throw UnexpectedTokenException(currentToken(), this->_source);
		} else if (!(error_code >= 300 && error_code <= 599)) {
			throw IncorrectValueException(currentToken(), this->_source);
		}

		error_codes.push_back(error_code);
		argument_count++;
		token = consume();
	}

	if (argument_count < 1 && !match(peek(), VALUE)) {
		throw InvalidNumberOfArgumentsException(error_page, this->_source);
	}

	String file = token.lexeme;

	std::vector<int>::iterator it = error_codes.begin();
	std::vector<int>::iterator end = error_codes.end();

	for (; it != end; it++) {
		directive_context.error_page.insert(std::make_pair(*it, file));
	}

	expect(SEMICOLON);
}


void	Parser::parseClientMaxBodySize(SharedDirectives& directive_context) {
	Token token = consume();

	// no value provided
	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	// value is not a number
	if (!std::isdigit(token.lexeme.c_str()[0])) {
		throw IncorrectValueException(token, this->_source);
	}

	// handle unit transition and unit check
	char* end = NULL;
	long body_size = std::strtol(token.lexeme.c_str(), &end, 10);
	// no unit is provided
	if (*end == '\0') {
		directive_context.client_max_body_size = body_size;
	}
	// unit is provided
	else {
		String unit = token.lexeme.substr(token.lexeme.find_first_of(*end));
		if (body_size == 0 && (unit == "k" || unit == "K" || unit == "m" || unit == "M")) {
			directive_context.client_max_body_size = 0;
		} else if (unit == "k" || unit == "K") {
			directive_context.client_max_body_size = body_size * 1024;
		} else if (unit == "m" || unit == "M") {
			directive_context.client_max_body_size = body_size * 1024 * 1024;
		} else {
			throw UnexpectedTokenException(token, unit, this->_source);
		}
	}

	// check if value is larger than 100m
	unsigned long max_allowed_size = 100 * 1024 * 1024;
	if (directive_context.client_max_body_size > max_allowed_size
		|| body_size == LONG_MAX) {
		throw ValueTooLargeException(token, this->_source);
	}

	expect(SEMICOLON);
}


void	Parser::parseClientBodyTempPath(SharedDirectives& directive_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	directive_context.client_body_temp_path = token.lexeme;
	expect(SEMICOLON);
}



void	Parser::parseAutoindex(SharedDirectives& directive_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	if (token.lexeme == "on") {
		directive_context.autoindex = true;
	} else if (token.lexeme == "off") {
		directive_context.autoindex = false;
	} else {
		throw InvalidValueExceptions(token, "autoindex", this->_source);
	}

	expect(SEMICOLON);
}



void	Parser::parseIndex(SharedDirectives& directive_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	while (match(peek(), VALUE)) {
		directive_context.index.push_back(token.lexeme);
		token = consume();
	}

	expect(SEMICOLON);
}



void	Parser::parseDavMethods(SharedDirectives& directive_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	while (match(token, VALUE)) {
		// ignore case sensitivity
		if (token.lexeme == "delete" || token.lexeme == "put") {
			String& lexeme = token.lexeme;
			std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::toupper);
		}

		if (token.lexeme == "DELETE" || token.lexeme == "PUT") {
			// match nginx rejecting duplicates
			std::set<String>& dav_methods = directive_context.dav_methods;
			if (dav_methods.find(token.lexeme) != dav_methods.end()) {
				throw DuplicatedValueException(token, this->_source);
			}

			dav_methods.insert(token.lexeme);

		} else if (token.lexeme == "off") {
			directive_context.dav_methods = std::set<String>();

			while (match(peek(), VALUE)) {
				consume();
				continue;
			}
			goto expect_semicolon;
		} else {
			throw InvalidValueExceptions(token, "dav_methods", this->_source);
		}

		if (!match(peek(), VALUE)) {
			goto expect_semicolon;
		}
		token = consume();
	} // while match value

expect_semicolon:
	expect(SEMICOLON);
}



void	Parser::parserCreateFullPutPath(SharedDirectives& directive_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	if (token.lexeme == "on") {
		directive_context.create_full_put_path = true;
	} else if (token.lexeme == "off") {
		directive_context.create_full_put_path = false;
	} else {
		throw InvalidValueExceptions(token, "autoindex", this->_source);
	}

	expect(SEMICOLON);
}

// -----------------------------------------------------------------
