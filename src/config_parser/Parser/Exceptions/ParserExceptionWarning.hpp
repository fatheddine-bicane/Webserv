#pragma once

#include <sstream>
#include <string>
#include "../../../Includes/colors.hpp"
#include "../../Scanner/Definitions/Token.hpp"
#include "../../Includes/ParserExceptions.hpp"

typedef std::string String;


class ParserExceptionWarning : public ParserException {
protected:
	String	generateForematedWarning(const Token& token,
								   const String& error_type,
								   const String& source) {
		std::stringstream ss;

		ss << YELLOW << "Parser Warning:" << token.line << ":"
			<< token.spaces + token.tabs*4 << ": "
			<< RESET << error_type << ": '"
			<< token.lexeme
			<< "'" << '\n';

		size_t end_of_line = source.find('\n', token.line_start);
		if (end_of_line == String::npos) {
			end_of_line = source.length();
		}
		String line = source.substr(token.line_start, end_of_line - token.line_start);
		ss << line << '\n';

		int count = 0;
		while (count < token.tabs) {
			ss << '\t';
			count++;
		}
		count = 1;
		while (count < token.spaces) {
			ss << ' ';
			count++;
		}
		ss << YELLOW << "^" << RESET;

		return ss.str();
	}

};


class ServerBlockIgnoredException : public ParserExceptionWarning {
public:
	ServerBlockIgnoredException(const Token& token,
							    const String& server_name,
							    const String& ip_port,
							    const String& source) {
		String error_type = "server block will be ignored, anothere server"
			" with the same 'server_name' directive: '" + server_name +
			"', and same IP-PORT combination: '" + ip_port + "'. In directive";

		this->_err = generateForematedWarning(token, error_type, source);
	}
};
