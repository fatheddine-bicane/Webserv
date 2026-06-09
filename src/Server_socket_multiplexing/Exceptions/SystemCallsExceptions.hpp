#pragma once

#include <cerrno>
#include <exception>
#include <sstream>
#include <vector>

#include "../../Includes/colors.hpp"
#include "../../Includes/Typedef.hpp"

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
	BindSysCallFailedException(Addresses::iterator& ip_port) {
		std::stringstream ss;
		ss << RED << "System call Error: " << RESET
		   << "bind() failed, address: '"
		   << ip_port->first << ":" << ip_port->second
		   << "' already in use, errno value: " << errno;
	}
};
