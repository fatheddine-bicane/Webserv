#include "../Definitions/Scanner.hpp"


Scanner::Scanner(int argc, char** argv) {

	// if config file not provided use default one
	String path;
	if (argc == 1) {
		path = "nginx/nginx.conf";
	} else {
		path = argv[1];
	}

	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + path);
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	this->source = new String(buffer.str());
	file.close();

	this->_start = 0;
	this->_current = 0;
	this->_line = 1;
	this->_spaces = 0;
	this->_tabs = 0;
	this->_line_start = 0;

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
	this->_keywords.insert(std::make_pair("server_name",           SERVER_NAME));
}

Scanner::~Scanner() {
	delete source;
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
			while (peek() != '\n' && !isAtEnd()) {
				consume();
			}
			break;

		// ignore white spaces
		case ' ' :
		case '\t':
		case '\r':
			break;

		// new lines
		case '\n':
			this->_line++;
			this->_spaces = 0;
			this->_tabs = 0;
			this->_line_start = this->_current;
			break;

		// token encountered is a word
		default:
			if (isValidChar(c)) {
				identifier();
			} else {
				throw UnexpectedCharacterException(generateErrorString());
			}
	}
}

void	Scanner::identifier() {
	while (isValidChar(peek())) {
		consume();
	}

	String text = this->source->substr(this->_start, this->_current - this->_start);

	std::map<String, TokenType>::iterator keyword;
	keyword = this->_keywords.find(text);

	if (keyword == this->_keywords.end()) {
		addToken(VALUE, text);
	} else {
		addToken(keyword->second);
	}
}

void	Scanner::addToken(TokenType token_type) {
	String lexeme = "END_OF_FILE";
	if (token_type != END_OF_FILE) {
		lexeme = this->source->substr(this->_start, this->_current - this->_start);
	}

	addToken(token_type, lexeme);
}

void	Scanner::addToken(TokenType token_type, String lexeme) {
	this->_tokens.push_back(
		Token(token_type, (lexeme), this->_line,
			  this->_spaces + this->_tabs*4, this->_spaces,
			  this->_tabs, this->_line_start));
}

String	Scanner::generateErrorString() {
	std::stringstream ss;

	ss << RED << "Error:" << this->_line << ":"
	   << this->_spaces + this->_tabs*4 << ": "
	   << RESET << "Unexpected character: '"
	   <<  this->source->at(this->_current - 1)
	   << "'" << '\n';

	int end_of_line = this->source->find('\n', this->_line_start);
	String line = this->source->substr(this->_line_start, end_of_line - this->_line_start);
	ss << line << '\n';

	int count = 0;
	while (count < this->_tabs) {
		ss << '\t';
		count++;
	}
	count = 1;
	while (count < this->_spaces) {
		ss << ' ';
		count++;
	}
	ss << RED << "^" << RESET;

	return ss.str();
}

bool	Scanner::isValidChar(char c) {

	return (std::isdigit(c) || std::isalpha(c) ||
			c == '_' || c == '-' || c == '/' || c == '.' || c == ':');
}

char	Scanner::consume() {
	char c = this->source->at(this->_current++);

	c == '\t' ? this->_tabs++ : this->_spaces++;

	return c;
}

char	Scanner::peek() {
	if (isAtEnd()) return '\0';
	return (this->source->at(this->_current));
}

bool	Scanner::isAtEnd() {
	return (this->_current >= static_cast<int>(this->source->length()));
}
