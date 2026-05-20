#pragma once

#include <exception>
#include <sstream>
#include <string>
#include "../../../Includes/colors.hpp"
#include "../../Scanner/Definitions/Token.hpp"

typedef std::string String;


class ParserException : public std::exception {
protected:
	std::string	generateForematedError(const Token& token,
									   const String& error_type,
									   const String& source) {
		std::stringstream ss;

		ss << RED << "Error:" << token._line << ":"
			<< token._spaces + token._tabs*4 << ": "
			<< RESET << error_type << ": '"
			<< token._lexeme
			<< "'" << '\n';

		size_t end_of_line = source.find('\n', token._line_start);
		if (end_of_line == String::npos) {
			end_of_line = source.length();
		}
		String line = source.substr(token._line_start, end_of_line - token._line_start);
		ss << line << '\n';

		int count = 0;
		while (count < token._tabs) {
			ss << '\t';
			count++;
		}
		count = 1;
		while (count < token._spaces) {
			ss << ' ';
			count++;
		}
		ss << RED << "^" << RESET;

		return ss.str();
	}

};

