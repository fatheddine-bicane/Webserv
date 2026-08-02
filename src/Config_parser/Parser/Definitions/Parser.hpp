#pragma once

#include <climits>
#include <vector>
#include <utility>
#include <map>
#include <cstdlib>
#include <algorithm>

#include "../Definitions/Directives.hpp"
#include "../../Scanner/Definitions/Token.hpp"
#include "../../Scanner/Definitions/TokenType.hpp"
#include "../Exceptions/ParserExceptionError.hpp"
#include "../Exceptions/ParserExceptionWarning.hpp"
#include "../../../Core_modules/Typedef.hpp"


class Parser {
private:
	String&				_source;
	int					_current;
	std::vector<Token>	_tokens;
	std::vector<std::pair<String, String> > _addresses;
	Servers				_servers;


// INFO: constructors
public:
	Parser(std::vector<Token> tokens, String& source);


// INFO: API
public:
	void	scanTokens();
	Servers& getServers();
	std::vector<std::pair<String, String> >& getAddresses();



// INFO: utility functions
private:
	void	scanToken();

	Token	consume();
	Token	currentToken();
	Token	previousToken();
	Token	peek();
	void	expect(TokenType token_type);
	bool	match(TokenType to_match);
	bool	match(Token token, TokenType to_match);
	bool	isAtEnd();
	bool	isDirective(TokenType token_type);
	bool	isHttpMethod(const String& method);
	bool	isServerBlockExist(String& ip_port, String& server_name);


// INFO: block directive parsers (context)
private:
	void	parseEvents();
	void	parseHttp();
	void	parseServer(SharedDirectives& directive_context);


// INFO: simple directive parsers
private:
	void	parseRoot(SharedDirectives& directive_context);
	void	parseErrorPage(SharedDirectives& directive_context);
	void	parseClientMaxBodySize(SharedDirectives& directive_context);
	void	parseClientBodyTempPath(SharedDirectives& directive_context);
	void	parseAutoindex(SharedDirectives& directive_context);
	void	parseIndex(SharedDirectives& directive_context);
	void	parseDavMethods(SharedDirectives& directive_context);
	String	parseServerName();
	void	parseReturn(std::pair<int, String>& return_d);
	void	parseAlias(Location& location_context);
	void	parseLimitExcept(Location& location_context);
	void	parseCgiPass(Location& location_context);
	void	parseLocation(Server& server);
	void	parseIp(const String& ip);
	void	parseService(const String& port);
	String	parseListen();

};
