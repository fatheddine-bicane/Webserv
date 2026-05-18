#include "../includes/Lexer.hpp"


Lexer::Lexer(const std::string& filePath) : _filePath(filePath) {
    this->_initKeywordMap();
}

Lexer::~Lexer(){}

void Lexer::_initKeywordMap(){
	this->_keywords["server"] = TOKEN_SERVER;
	this->_keywords["location"] = TOKEN_LOCATION;
	this->_keywords["listen"] = TOKEN_LISTEN;
	this->_keywords["server_name"] = TOKEN_SERVER_NAME;
	this->_keywords["root"] = TOKEN_ROOT;
	this->_keywords["index"] = TOKEN_INDEX;
	this->_keywords["autoindex"] = TOKEN_AUTOINDEX;
	this->_keywords["error_page"] = TOKEN_ERROR_PAGE;
	this->_keywords["client_max_body_size"] = TOKEN_MAX_BODY_SIZE;
	this->_keywords["limit_except"] = TOKEN_LIMIT_EXCEPT;
	this->_keywords["alias"] = TOKEN_ALIAS;
	this->_keywords["return"] = TOKEN_RETURN;
	this->_keywords["dav_methods"] = TOKEN_DAV_METHODS;
	this->_keywords["client_body_temp_path"] = TOKEN_CLIENT_BODY_TEMP_PATH;
	this->_keywords["create_full_put_path"] = TOKEN_CREATE_FULL_PUT_PATH;
}

std::vector<Token> Lexer::tokenize() {
	std::ifstream file(this->_filePath.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Lexer Error: Could not open file " + this->_filePath);
	}

	std::vector<Token> labeledTokens;
	std::string currentWord = "";
	char ch;

	while (file.get(ch)) {
		// handle comments
		if (ch == '#') {
			while (file.get(ch) && ch != '\n' && ch != '\r');
			if (!currentWord.empty()) {
				Token tok = { this->_keywords.count(currentWord) ? this->_keywords[currentWord] : TOKEN_IDENTIFIER, currentWord };
				labeledTokens.push_back(tok);
				currentWord.clear();
			}
			continue;
		}

		// handle delimiters
		if (ch == '{' || ch == '}' || ch == ';') {
			if (!currentWord.empty()) {
				Token tok = { this->_keywords.count(currentWord) ? this->_keywords[currentWord] : TOKEN_IDENTIFIER, currentWord };
				labeledTokens.push_back(tok);
				currentWord.clear();
			}

			Token structTok;
			structTok.value = std::string(1, ch);
			if (ch == '{')      structTok.type = TOKEN_LBRACE;
			else if (ch == '}') structTok.type = TOKEN_RBRACE;
			else                structTok.type = TOKEN_SEMICOLON;

			labeledTokens.push_back(structTok);
			continue;
		}

		// handle white sapce
		if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
			if (!currentWord.empty()) {
				Token tok = { this->_keywords.count(currentWord) ? this->_keywords[currentWord] : TOKEN_IDENTIFIER, currentWord };
				labeledTokens.push_back(tok);
				currentWord.clear();
			}
			continue;
		}
		currentWord += ch;
	}

	// Flush any trailing word hanging out at EOF
	if (!currentWord.empty()) {
		Token tok = { this->_keywords.count(currentWord) ? this->_keywords[currentWord] : TOKEN_IDENTIFIER, currentWord };
		labeledTokens.push_back(tok);
	}

	// Append standard End of File token
	Token eofToken = {TOKEN_EOF, ""};
	labeledTokens.push_back(eofToken);

	file.close();
	return labeledTokens;
}
