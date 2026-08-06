#pragma once

#include <sstream>
#include "../../../Core_modules/colors.hpp"
#include "../../Scanner/Definitions/Token.hpp"
#include "../../Includes/ParserExceptions.hpp"
#include "../../../Core_modules/Typedef.hpp"


enum Port_error{
	PRIVILEGED_PORT, INVALID_PORT_VALUE
};



class ParserExceptionError : public ParserException{
protected:
	String	generateForematedError(const Token& token,
								   const String& error_type,
								   const String& source) {
		std::stringstream ss;

		ss << RED << "Parser Error:" << token.line << ":"
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
		ss << RED << "^" << RESET;

		return ss.str();
	}

};



class DuplicatedDirectiveException : public ParserExceptionError {
public:
	DuplicatedDirectiveException (const Token& token, const String& source) {
		this->_err = generateForematedError(token, "duplicated directive", source);
	}
};


class ExpectedTokenException : public ParserExceptionError {
public:
	ExpectedTokenException(const Token& token, const String& missing_token, String& source) {

		String error_type = "expected token '" + missing_token + "' after";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class IncorrectValueException : public ParserExceptionError {
public:
	IncorrectValueException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "incorrect value", source);
	}
};



class InvalidNumberOfArgumentsException : public ParserExceptionError {
public:
	InvalidNumberOfArgumentsException(const Token& token, const String& source) {
		String error_type = "invalid number of arguments in directive";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class UnexpectedTokenException : public ParserExceptionError {
public:
	UnexpectedTokenException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "unexpected token", source);
	}

	UnexpectedTokenException(Token& token, const String& token_value,  const String& source) {
		if (!token_value.empty()) {
			token.lexeme = token_value;
		}
		this->_err = generateForematedError(token, "unexpected token", source);
	}
};



class ValueTooLargeException : public ParserExceptionError {
public:
	ValueTooLargeException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "value too large", source);
	}
};



class InvalidValueExceptions : public ParserExceptionError {
public:
	InvalidValueExceptions(Token& token, const String& directive, const String& source) {
		String value = token.lexeme;
		token.lexeme = directive;
		String error_type = "invalid value '" + value + "' in directive";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class DuplicatedValueException : public ParserExceptionError {
public:

	DuplicatedValueException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "duplicate value", source);
	}
};



class DirectiveNotAllowedHereException : public ParserExceptionError {
public:
	DirectiveNotAllowedHereException (const Token& token, const String& source) {
		String error_type = "directive not allowed in this context";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class UnsupportedCgiScriptType : public ParserExceptionError {
public:
	UnsupportedCgiScriptType(const Token& token, const String& source) {
		String error_type = "unsupported cgi script format";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class InvalidPortNumberException : public ParserExceptionError {
public:
	InvalidPortNumberException(Token& token,
							   const String& port_value,
							   Port_error port_error,
							   const String& source) {
		String error_type;
		std::stringstream ss;
		if (port_error == PRIVILEGED_PORT) {
			error_type = "attempted connection to a privileged port: '" + port_value
			+ "' in directive";
		} else if (INVALID_PORT_VALUE){
			error_type = "invalid port value: '" + port_value
			   + "'. Port value must be a number in range: (1024 - 65535). In directive";
		}
		token.lexeme = "listen";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class InvalidIpAddressValueException : public ParserExceptionError {
public:
	InvalidIpAddressValueException(Token& token,
								const String& address,
								const String& source) {
		String error_type = "invalid ip address value: " + address + " in directive";
		token.lexeme = "listen";
		this->_err = generateForematedError(token, error_type, source);
	}
};



class BlockDirectiveViolationException : public ParserExceptionError {
public:

	BlockDirectiveViolationException(const Token& token,
									 const String& directive,
									 const String& source) {
		String error_type = "Once a '" + directive + "' block is defined, "
			"only additional '" + directive + "' blocks are allowed in this"
			" context until the current scope is closed.";

		this->_err = generateForematedError(token, error_type, source);
	}
};



class NoLocationDirectiveFoundException : public ParserExceptionError {
public:

	NoLocationDirectiveFoundException(const Token& token, const String& source) {
		String error_type = "A server block must contain at least "
							"one location block directive to route "
							"incoming traffic properly.";

		this->_err = generateForematedError(token, error_type, source);
	}
};
