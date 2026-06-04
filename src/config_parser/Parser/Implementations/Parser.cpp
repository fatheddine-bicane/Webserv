#include "../Definitions/Parser.hpp"


// INFO: constructors
// ---------------------------------------------------------

Parser::Parser(std::vector<Token> tokens, String& source) 
	: _source(source) {
	this->_current = 0;
	this->_tokens = tokens;
}

// ---------------------------------------------------------



// INFO: API
// ---------------------------------------------------------

void	Parser::scanTokens() {

	while (!isAtEnd()) {
		scanToken();
	}
}


Servers&	Parser::getServers() {
	return this->_servers;
}


std::vector<std::pair<String, String> >& Parser::getAddresses() {
	return this->_addresses;
}

// ---------------------------------------------------------



// INFO: utility functions
// -----------------------------------------------------------------

void	Parser::scanToken() {
	Token token = consume();

	switch (token.type) {
		case EVENTS:
			parseEvents();

			if (match(peek(), EVENTS)) {
				throw DuplicatedDirectiveException(peek(), this->_source);
			} else if (!match(peek(), HTTP)) {
				throw ExpectedTokenException(token, "http", this->_source);
			}
			break;
		case HTTP:
			parseHttp();

			if (match(peek(), HTTP)) {
				throw DuplicatedDirectiveException(peek(), this->_source);
			} else if (!match(peek(), END_OF_FILE)) {
				throw ExpectedTokenException(token, "end_of_file", this->_source);
			}

			break;


		default:
			throw UnexpectedTokenException(token, this->_source);
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
		case SERVER_NAME:
			return true;

		default:
			return false;
	}
}



bool	Parser::isHttpMethod(const String& method) {
	return (method == "GET"
		 || method == "POST"
		 || method == "DELETE"
		 || method == "PUT");
}

// -----------------------------------------------------------------



// INFO: block directive parsers (context)
// -----------------------------------------------------------------

void	Parser::parseEvents() {
	expect(CONTEXT_START);
	expect(CONTEXT_END);
}


void	Parser::parseHttp() {
	SharedDirectives directive_context;
	expect(CONTEXT_START);
	Token token = consume();
	bool server_block_appeard = false;

	while (!match(token, CONTEXT_END)) {
		if (server_block_appeard && !match(token, SERVER)) {
			if (isDirective(token.type)) {
				throw BlockDirectiveViolationException(token,"server", this->_source);
			} else {
				throw UnexpectedTokenException(token, this->_source);
			}
		}

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
			case SERVER:
				server_block_appeard = true;
				parseServer(directive_context);
				break;

			// end of file reached without closing the context
			case END_OF_FILE:
				throw ExpectedTokenException(previousToken(), "}", this->_source);

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



void	Parser::parseServer(SharedDirectives& directive_context) {
	expect(CONTEXT_START);

	Server server_directive;
	server_directive.shared_directives = directive_context;
	String server_name;
	bool server_name_parsed = false;
	Token server_token = previousToken();
	bool location_block_appered = false;

	Token token = consume();

	while (!match(token, CONTEXT_END)) {
		if (location_block_appered && !match(token, LOCATION)) {
			if (isDirective(token.type)) {
				throw BlockDirectiveViolationException(token, "location", this->_source);
			} else {
				throw UnexpectedTokenException(token, this->_source);
			}
		}

		switch (token.type) {
			case SERVER_NAME:
				parseServerName(server_name);
				server_name_parsed = true;
				break;
			case ROOT: parseRoot(server_directive.shared_directives); break;
			case ERROR_PAGE:
				parseErrorPage(server_directive.shared_directives);
				break;
			case CLIENT_MAX_BODY_SIZE:
				parseClientMaxBodySize(server_directive.shared_directives);
				break;
			case CLIENT_BODY_TEMP_PATH:
				parseClientBodyTempPath(server_directive.shared_directives);
				break;
			case AUTOINDEX: parseAutoindex(server_directive.shared_directives);
				break;
			case INDEX: parseIndex(server_directive.shared_directives); break;
			case DAV_METHODS:
				parseDavMethods(server_directive.shared_directives);
				break;
			case CREATE_FULL_PUT_PATH:
				parserCreateFullPutPath(server_directive.shared_directives);
				break;
			case RETURN: parseReturn(server_directive.return_d); break;
			case LOCATION:
				location_block_appered = true;
				parseLocation(server_directive);
				break;
			case LISTEN: parseListen(); break;

			// end of file reached without closing the context
			case END_OF_FILE:
				throw ExpectedTokenException(previousToken(), "}", this->_source);

			default:
				if (isDirective(token.type)) {
					throw DirectiveNotAllowedHereException(token, this->_source);
				} else {
					throw UnexpectedTokenException(token, "", this->_source);
				}
		}

		token = consume();
	} // while match CONTEXT_END

	if (server_name_parsed == true) {
		this->_servers.insert(std::make_pair(server_name, server_directive));
	} else {
		throw ServerNameMissingException(server_token, this->_source);
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



void	Parser::parseServerName(String& server_name) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	server_name = token.lexeme;
	expect(SEMICOLON);
}



void	Parser::parseReturn(std::pair<int, String>& return_d) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	char* end = NULL;
	long error_value = std::strtol(token.lexeme.c_str(), &end, 10);
	if (*end != '\0' || !(error_value >= 300 && error_value <= 599)) {
		throw InvalidValueExceptions(token, "return", this->_source);
	}

	return_d.first = error_value;

	token = consume();
	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	return_d.second = token.lexeme;

	expect(SEMICOLON);
}



void	Parser::parseAlias(Location& location_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	location_context.alias = token.lexeme;
	expect(SEMICOLON);
}



void	Parser::parseLimitExcept(Location& location_context) {
	std::set<String>& limit_except = location_context.limit_except;
	Token token = consume();

	if (!match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	while (match(token, VALUE)) {
		if (!isHttpMethod(token.lexeme)) {
			throw InvalidValueExceptions(token, "limit_except", this->_source);
		}

		if (limit_except.find(token.lexeme) != limit_except.end()) {
			throw DuplicatedValueException(token, this->_source);
		}

		location_context.limit_except.insert(token.lexeme);
		token = consume();
	}

	if (!match(token, CONTEXT_START)) {
		throw UnexpectedTokenException(token, this->_source);
	}

	token = consume();
	if (token.lexeme != "deny") {
		throw UnexpectedTokenException(token, this->_source);
	}

	token = consume();
	if (token.lexeme != "all") {
		throw UnexpectedTokenException(token, this->_source);
	}

	expect(SEMICOLON);
	expect(CONTEXT_END);
}



void	Parser::parseCgiPass(Location& location_context) {
	Token token = consume();

	if (match(token, SEMICOLON) || !match(token, VALUE) || !match(peek(), VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}

	if (token.lexeme == ".py" || token.lexeme == ".js") {
		String extention = token.lexeme;


		std::vector<std::pair<String, String> >::iterator it;
		std::vector<std::pair<String, String> >::iterator end;
		it = location_context.cgi_pass.begin();
		end = location_context.cgi_pass.end();
		for (; it != end; it++) {
			if (it->first == extention) {
				throw DuplicatedValueException(token, this->_source);
			}
		}

		token = consume();
		location_context.cgi_pass.push_back(std::make_pair(extention, token.lexeme));
	} else {
		throw UnsupportedCgiScriptType(token, this->_source);
	}

	expect(SEMICOLON);
}



void	Parser::parseLocation(Server& server) {
	Location location_directive;
	location_directive.shared_directives = server.shared_directives;

	Token token = consume();
	if (!match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(previousToken(), this->_source);
	}
	location_directive.path = token.lexeme;

	expect(CONTEXT_START);

	token = consume();
	while (!match(token, CONTEXT_END)) {
		switch (token.type) {
			case ROOT: parseRoot(location_directive.shared_directives); break;
			case ERROR_PAGE:
				parseErrorPage(location_directive.shared_directives);
				break;
			case CLIENT_MAX_BODY_SIZE:
				parseClientMaxBodySize(location_directive.shared_directives);
				break;
			case CLIENT_BODY_TEMP_PATH:
				parseClientBodyTempPath(location_directive.shared_directives);
				break;
			case AUTOINDEX: parseAutoindex(location_directive.shared_directives);
				break;
			case INDEX: parseIndex(location_directive.shared_directives); break;
			case DAV_METHODS:
				parseDavMethods(location_directive.shared_directives);
				break;
			case CREATE_FULL_PUT_PATH:
				parserCreateFullPutPath(location_directive.shared_directives);
				break;
			case RETURN: parseReturn(location_directive.return_d); break;
			case ALIAS: parseAlias(location_directive); break;
			case LIMIT_EXCEPT: parseLimitExcept(location_directive); break;
			case CGI_PASS: parseCgiPass(location_directive); break;

			// end of file reached without closing the context
			case END_OF_FILE:
				throw ExpectedTokenException(previousToken(), "}", this->_source);

			default:
				if (isDirective(token.type)) {
					throw DirectiveNotAllowedHereException(token, this->_source);
				} else {
					throw UnexpectedTokenException(token, "", this->_source);
				}
		}

		token = consume();
	} // while match CONTEXT_END

	server.locations.push_back(location_directive);
}



void	Parser::parseIp(const String& ip) {
	std::stringstream ss(ip);
	Token token = currentToken();

	for (int i = 0; i < 5; i++) {
		String octet_str;
		std::getline(ss, octet_str, '.');
		if (i == 4 ) {
			if (!octet_str.empty()) {
				throw InvalidIpAddressValueException(token, ip, this->_source);
			} else {
				break;
			}
		} else if (octet_str.empty()) {
			throw InvalidIpAddressValueException(token, ip, this->_source);
		}

		char* end = NULL;
		long octect_value = std::strtol(octet_str.c_str(), &end, 10);
		if (*end != '\0') {
			throw InvalidIpAddressValueException(token, ip, this->_source);
		}

		if (!(octect_value >= 0 && octect_value <= 255)) {
			throw InvalidIpAddressValueException(token, ip, this->_source);
		}
	}
}



void	Parser::parseService(const String& service) {
	Token token = currentToken();

	char* end = NULL;
	long service_value = strtol(service.c_str(), &end, 10);
	if (*end != '\0') {
		throw InvalidPortNumberException(token, service,
								   INVALID_PORT_VALUE, this->_source);
	}
	if (service_value >= 0 && service_value <= 1023) {
		throw InvalidPortNumberException(token, service,
								   PRIVILEGED_PORT, this->_source);
	} else if (service_value < 0 || service_value > 65535) {
		throw InvalidPortNumberException(token, service,
								   INVALID_PORT_VALUE, this->_source);
	}
}



void	Parser::parseListen() {
	Token token = consume();

	if (!match(token, VALUE)) {
		throw InvalidNumberOfArgumentsException(token, this->_source);
	}

	String ip;
	String service;
	size_t pos = token.lexeme.find(':');
	if (pos != String::npos) {
		ip = token.lexeme.substr(0, pos);
		service = token.lexeme.substr(pos + 1);
	} else if (token.lexeme.find('.') != String::npos) {
		ip = token.lexeme;
		service = "8080";
	} else {
		ip = "0.0.0.0";
		service = token.lexeme;
	}

	parseIp(ip);
	parseService(service);

	std::pair<String, String> address = std::make_pair(ip, service);

	std::vector<std::pair<String, String> >::iterator begin;
	std::vector<std::pair<String, String> >::iterator last;
	std::vector<std::pair<String, String> >::iterator it;
	begin = this->_addresses.begin();
	last = this->_addresses.end();
	it = std::find(begin, last, address);
	if (it == last) {
		this->_addresses.push_back(std::make_pair(ip, service));
	}

	expect(SEMICOLON);
}

// -----------------------------------------------------------------
