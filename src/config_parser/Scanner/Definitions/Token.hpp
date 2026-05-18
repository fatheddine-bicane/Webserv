#pragma once

#include <string>
#include <iostream>
#include "./TokenType.hpp"

typedef std::string String;


class Token {
private:
	TokenType	_token_type;
	String		_lexeme;
	int			_line;
	int			_position;

public:
	Token(TokenType token_type, String lexeme, int line, int position);
	String	toString();
};
