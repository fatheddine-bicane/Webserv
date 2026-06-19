#pragma once

#include <exception>

#include "../../Core_modules/HTTPStatus.hpp"

class ProcessRequestException : public std::exception {
public:
	HTTPStatus	status_code;

	ProcessRequestException(HTTPStatus status_code) {
		this->status_code = status_code;
	}

	// dummy wont be nedded
	const char * what() const throw() {
		return "";
	}
	
};
