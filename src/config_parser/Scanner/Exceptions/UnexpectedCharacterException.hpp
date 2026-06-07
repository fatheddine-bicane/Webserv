#pragma once

#include <string>

#include "../../Includes/ParserExceptions.hpp"

typedef std::string String;


class UnexpectedCharacterException : public ParserException {
public:
	UnexpectedCharacterException(const String& err) {
		this->_err = err;
	}
};
