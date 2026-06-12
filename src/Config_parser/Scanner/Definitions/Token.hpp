#pragma once

#include <iostream>
#include "./TokenType.hpp"

#include "../../../Core_modules/Typedef.hpp"


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
