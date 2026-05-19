#include "../Definitions/Token.hpp"
#include <sstream>


Token::	Token(TokenType token_type, String lexeme, int line,
		  int position, int spaces, int tabs, int line_start) {
	this->_token_type = token_type;
	this->_lexeme = lexeme;
	this->_line = line;
	this->_position = position;
	this->_spaces = spaces;
	this->_tabs = tabs;
	this->_line_start = line_start;
}

String	Token::toString(String& source) {
	(void) this->_line;
	(void) this->_position;
	(void) this->_token_type;
	(void) this->_spaces;
	(void) this->_tabs;
	(void) this->_line_start;

	std::stringstream ss;
	ss << this->_lexeme << " position: " << this->_position << " line: " << this->_line << '\n';
	int end_of_line = source.find('\n', this->_line_start);
	String line = source.substr(this->_line_start, end_of_line - this->_line_start);
	ss << "LINE: " << line << '\n';

	return ss.str();
}
