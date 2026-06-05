#pragma once

#include <string>
#include <iostream>
#include "./TokenType.hpp"

typedef std::string String;


class Token {
public:
	TokenType	type;
	String		lexeme;
	int			line;
	int			position;
	int			spaces;
	int			tabs;
	int			line_start;

public:
	Token(TokenType token_type, String lexeme, int line,
		  int position, int spaces, int tabs, int line_start);
	String	toString(String& source);
};
