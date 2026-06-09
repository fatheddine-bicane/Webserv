#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdexcept>

#include "./TokenType.hpp"
#include "./Token.hpp"
#include "../Exceptions/UnexpectedCharacterException.hpp"
#include "../../../Includes/colors.hpp"
#include "../../../Includes/Typedef.hpp"


class Scanner {
private:
	int		_start;
	int		_current;
	int		_line;
	int		_spaces;
	int		_tabs;
	int		_line_start;
	std::map<String, TokenType>	_keywords;
	std::vector<Token> _tokens;

public:
	String*	source;

public:
	Scanner(int argc, char** argv);
	std::vector<Token>	scanTokens();
	~Scanner();

private:
	void	scanToken();
	void	identifier();
	void	addToken(TokenType token_type);
	void	addToken(TokenType token_type, String lexeme);
	String	generateErrorString();
	bool	isValidChar(char c);
	char	consume();
	char	peek();
	bool	isAtEnd();
};
