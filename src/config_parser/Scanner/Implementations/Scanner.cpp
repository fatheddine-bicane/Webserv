#include "../Definitions/Scanner.hpp"
#include <cctype>
#include <map>
#include <vector>


Scanner::Scanner(const String& source) {
	this->_source = source;
	this->_start = 0;
	this->_current = 0;
	this->_line = 1;

	this->_keywords.insert(std::make_pair("events",                EVENTS));
	this->_keywords.insert(std::make_pair("http",                  HTTP));
	this->_keywords.insert(std::make_pair("server",                SERVER));
	this->_keywords.insert(std::make_pair("location",              LOCATION));
	this->_keywords.insert(std::make_pair("limit_except",          LIMIT_EXCEPT));
	this->_keywords.insert(std::make_pair("listen",                LISTEN));
	this->_keywords.insert(std::make_pair("error_page",            ERROR_PAGE));
	this->_keywords.insert(std::make_pair("root",                  ROOT));
	this->_keywords.insert(std::make_pair("client_max_body_size",  CLIENT_MAX_BODY_SIZE));
	this->_keywords.insert(std::make_pair("autoindex",             AUTOINDEX));
	this->_keywords.insert(std::make_pair("index",                 INDEX));
	this->_keywords.insert(std::make_pair("alias",                 ALIAS));
	this->_keywords.insert(std::make_pair("return",                RETURN));
	this->_keywords.insert(std::make_pair("dav_methods",           DAV_METHODS));
	this->_keywords.insert(std::make_pair("client_body_temp_path", CLIENT_BODY_TEMP_PATH));
	this->_keywords.insert(std::make_pair("create_full_put_path",  CREATE_FULL_PUT_PATH));
	this->_keywords.insert(std::make_pair("cgi_pass",              CGI_PASS));
}

std::vector<Token>	Scanner::scanTokens() {
	while (!isAtEnd()) {
		this->_start = this->_current;
		scanToken();
	}

	addToken(END_OF_FILE);
	return this->_tokens;
}

void	Scanner::scanToken() {
	char c = consume();
	
	switch (c) {
		// single character token
		case ';': addToken(SEMICOLON); break;
		case '{': addToken(CONTEXT_START); break;
		case '}': addToken(CONTEXT_END); break;

		// comments
		case '#':
			while (peek() != '\n' && !isAtEnd())
				consume();

		// ignore white spaces
		case ' ' :
		case '\t':
		case '\r':
			break;

		// new lines
		case '\n': this->_line++; break;

		// token encountered is a word
		default:
			if (isValidChar(c)) {
				identifier();
			} else {
				throw UnexpectedCharacterException("Unexpected character");
			}
	}
}

void	Scanner::identifier() {
	while (isValidChar(peek())) {
		consume();
	}

	String text = this->_source.substr(this->_start, this->_current - this->_start);

	std::map<String, TokenType>::iterator keyword;
	keyword = this->_keywords.find(text);

	if (keyword == this->_keywords.end()) {
		addToken(VALUE, text);
	} else {
		addToken(keyword->second);
	}
}

void	Scanner::addToken(TokenType token_type) {
	addToken(token_type, "");
}

void	Scanner::addToken(TokenType token_type, String lexeme) {
	// TODO: set the correct position index
	this->_tokens.push_back(
		Token(token_type, lexeme, this->_line, -1));
}

bool	Scanner::isValidChar(char c) {
	return (std::isdigit(c) || std::isalpha(c) ||
			c == '_' || c == '-' || c == '/' || c == '.');
}

char	Scanner::consume() {
	return (this->_source.at(this->_current++));
}

char	Scanner::peek() {
	if (isAtEnd()) return '\0';
	return (this->_source.at(this->_current));
}

bool	Scanner::isAtEnd() {
	return (this->_current >= static_cast<int>(this->_source.length()));
}
