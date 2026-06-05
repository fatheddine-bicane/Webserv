#include "../Definitions/Token.hpp"
#include <sstream>


Token::	Token(TokenType token_type, String lexeme, int line,
		  int position, int spaces, int tabs, int line_start) {
	this->type = token_type;
	this->lexeme = lexeme;
	this->line = line;
	this->position = position;
	this->spaces = spaces;
	this->tabs = tabs;
	this->line_start = line_start;
}

String	Token::toString(String& source) {
	(void) this->line;
	(void) this->position;
	(void) this->type;
	(void) this->spaces;
	(void) this->tabs;
	(void) this->line_start;

	std::stringstream ss;
	ss << this->lexeme << " position: " << this->position << " line: " << this->line << '\n';
	int end_of_line = source.find('\n', this->line_start);
	String line = source.substr(this->line_start, end_of_line - this->line_start);
	ss << "LINE: " << line << '\n';

	return ss.str();
}
