#pragma once

#include "../../Includes/ParserExceptions.hpp"
#include "../../../Includes/Typedef.hpp"


class UnexpectedCharacterException : public ParserException {
public:
	UnexpectedCharacterException(const String& err) {
		this->_err = err;
	}
};
