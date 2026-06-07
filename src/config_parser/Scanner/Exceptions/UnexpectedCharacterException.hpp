#pragma once

#include <string>

#include "../../Includes/ParserExceptions.hpp"


class UnexpectedCharacterException : public ParserException {
public:
	UnexpectedCharacterException(const std::string& err) {
		this->_err = err;
	}
};
