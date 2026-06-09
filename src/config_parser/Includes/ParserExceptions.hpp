#pragma once

#include <exception>
#include "../../Includes/Typedef.hpp"



class ParserException : public std::exception {
protected:
	String	_err;


public:
	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ParserException() throw() {}
};
