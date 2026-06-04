#pragma once

#include <exception>
#include <sstream>
#include <string>
#include <algorithm>
#include <set>
#include "../../../Includes/colors.hpp"
#include "../../Scanner/Definitions/Token.hpp"

typedef std::string String;


enum Port_error{
	PRIVILEGED_PORT, INVALID_PORT_VALUE
};



class ParserException : public std::exception {
protected:
	std::string	generateForematedError(const Token& token,
									   const String& error_type,
									   const String& source) {
		std::stringstream ss;

		ss << RED << "Error:" << token.line << ":"
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



class DuplicatedDirectiveException : public ParserException {
private:
	std::string	_err;

public:
	DuplicatedDirectiveException (const Token& token, const String& source) {
		this->_err = generateForematedError(token, "duplicated directive", source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~DuplicatedDirectiveException () throw() {}
};


class ExpectedTokenException : public ParserException {
private:
	std::string	_err;

public:
	ExpectedTokenException(const Token& token, const String& missing_token, String& source) {

		String error_type = "expected token '" + missing_token + "' after";
		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ExpectedTokenException() throw() {}
};



class IncorrectValueException : public ParserException {
private:
	std::string	_err;

public:
	IncorrectValueException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "incorrect value", source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~IncorrectValueException() throw() {}
};



class InvalidNumberOfArgumentsException : public ParserException {
private:
	std::string	_err;

public:
	InvalidNumberOfArgumentsException(const Token& token, const String& source) {
		String error_type = "invalid number of arguments in directive";
		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~InvalidNumberOfArgumentsException() throw() {}
};



class UnexpectedTokenException : public ParserException {
private:
	std::string	_err;

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

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~UnexpectedTokenException() throw() {}
};



class ValueTooLargeException : public ParserException {
private:
	std::string	_err;

public:
	ValueTooLargeException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "value too large", source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ValueTooLargeException() throw() {}
};



class InvalidValueExceptions : public ParserException {
private:
	std::string	_err;

public:
	InvalidValueExceptions(Token& token, const String& directive, const String& source) {
		String value = token.lexeme;
		token.lexeme = directive;
		String error_type = "invalid value '" + value + "' in directive";
		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~InvalidValueExceptions() throw() {}
};



class DuplicatedValueException : public ParserException {
private:
	std::string	_err;

public:

	DuplicatedValueException(const Token& token, const String& source) {
		this->_err = generateForematedError(token, "duplicate value", source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~DuplicatedValueException() throw() {}
};



class DirectiveNotAllowedHereException : public ParserException {
private:
	std::string	_err;

public:
	DirectiveNotAllowedHereException (const Token& token, const String& source) {
		String error_type = "directive not allowed in this context";
		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~DirectiveNotAllowedHereException() throw() {}
};



class UnsupportedCgiScriptType : public ParserException {
private:
	std::string	_err;

public:
	UnsupportedCgiScriptType(const Token& token, const String& source) {
		String error_type = "unsupported cgi script format";
		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~UnsupportedCgiScriptType() throw() {}
};



class InvalidPortNumberException : public ParserException {
private:
	std::string	_err;

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

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~InvalidPortNumberException() throw() {}

};



class InvalidIpAddressValueException : public ParserException {
private:
	std::string	_err;

public:
	InvalidIpAddressValueException(Token& token,
								const String& address,
								const String& source) {
		String error_type = "invalid ip address value: " + address + " in directive";
		token.lexeme = "listen";
		this->_err = generateForematedError(token, error_type, source);
	}


	const char * what() const throw() {
		return this->_err.c_str();
	}

	~InvalidIpAddressValueException() throw() {}
};



class BlockDirectiveViolationException : public ParserException {
private:
	std::string	_err;

public:

	BlockDirectiveViolationException(const Token& token,
									 const String& directive,
									 const String& source) {
		String error_type = "Once a '" + directive + "' block is defined, "
			"only additional '" + directive + "' blocks are allowed in this"
			" context until the current scope is closed.";

		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~BlockDirectiveViolationException() throw() {}
};



class ServerNameMissingException : public ParserException {
private:
	std::string	_err;

public:

	ServerNameMissingException(Token& token, const String& source) {
		String error_type = "directive 'server_name' is missing in directive";
		this->_err = generateForematedError(token, error_type, source);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ServerNameMissingException() throw() {}

};
