#pragma once

#include <string>
#include <vector>
#include <map>
#include "./TokenType.hpp"
#include "./Token.hpp"
#include "../Exceptions/UnexpectedCharacterException.hpp"
#include "../../../Includes/colors.hpp"

typedef std::string String;


class Scanner {
private:
	String&	_source;
	int		_start;
	int		_current;
	int		_line;
	int		_spaces;
	int		_tabs;
	int		_line_start;
	std::map<String, TokenType>	_keywords;
	std::vector<Token> _tokens;

public:
	Scanner(String& source);
	std::vector<Token>	scanTokens();

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
