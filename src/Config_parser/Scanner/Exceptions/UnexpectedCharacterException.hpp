#pragma once

#include "../../Includes/ParserExceptions.hpp"
#include "../../../Core_modules/Typedef.hpp"


class UnexpectedCharacterException : public ParserException {
public:
	UnexpectedCharacterException(const String& err) {
		this->_err = err;
	}
};
