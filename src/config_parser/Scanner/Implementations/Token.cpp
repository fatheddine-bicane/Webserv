#include "../Definitions/Token.hpp"
#include <sstream>


Token::	Token(TokenType token_type, String lexeme, int line, int position) {
	this->_token_type = token_type;
	this->_lexeme = lexeme;
	this->_line = line;
	this->_position = position;
}

String	Token::toString() {
	(void) this->_line;
	(void) this->_position;
	(void) this->_token_type;

	std::stringstream ss;

	ss << this->_lexeme << " position: " << this->_position << " line: " << this->_line << '\n';

	return ss.str();
}
