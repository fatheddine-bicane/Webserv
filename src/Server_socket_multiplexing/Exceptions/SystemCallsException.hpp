#pragma once

#include <cerrno>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include "../../Includes/colors.hpp"

typedef std::string String;
typedef std::vector<std::pair<String, String> > Addresses;


class SystemCallsFailedException : public std::exception {
protected:
	String	_err;


protected:
	String	generateErrorMessage(const String& sys_call) {
		std::stringstream ss;
		ss << RED << "System call Error: " << RESET
		   << sys_call << " failed, errno value: " << errno;

		return ss.str();
	}

public:
	SystemCallsFailedException() {}

	SystemCallsFailedException(const String& sys_call) {
		this->_err = generateErrorMessage(sys_call);
	}

	const char*	what() const throw() {
		return this->_err.c_str();
	}

	~SystemCallsFailedException() throw() {}
};



class BindSysCallFailedException : public SystemCallsFailedException {
public:
	BindSysCallFailedException(Addresses::iterator& it) {
		std::stringstream ss;
		ss << RED << "System call Error: " << RESET
		   << "bind() failed, address: '"
		   << it->first << ":" << it->second
		   << "' already in use, errno value: " << errno;
	}
};
