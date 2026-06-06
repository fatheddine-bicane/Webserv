#pragma once

#include <exception>
#include <string>

typedef std::string String;


class ParserException : public std::exception {
protected:
	String	_err;


public:
	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ParserException() throw() {}
};
