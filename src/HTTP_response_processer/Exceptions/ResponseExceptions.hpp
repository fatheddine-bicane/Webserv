#pragma once

#include <exception>
#include <sstream>
#include "../../Core_modules/Typedef.hpp"
#include "../../Core_modules/colors.hpp"


class ClientSocketErrorException : public std::exception {

private:
	String _err;

public:
	ClientSocketErrorException() {
		std::stringstream ss;

		ss << RED "Underlying socket error:" RESET 
		   << " An unexpected network error occurred.";

		this->_err = ss.str();
	}

	// dummy wont be nedded
	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ClientSocketErrorException() throw() {}
};
