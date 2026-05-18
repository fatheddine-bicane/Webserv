#pragma once


#include <exception>
#include <string>

class UnexpectedCharacterException : std::exception {
private:
	std::string _err;

public:
	UnexpectedCharacterException(const std::string& err) {
		this->_err = err;
	}

	const char* what() const throw() {
		return this->_err.c_str();
	}

	~UnexpectedCharacterException() throw() {}
};
