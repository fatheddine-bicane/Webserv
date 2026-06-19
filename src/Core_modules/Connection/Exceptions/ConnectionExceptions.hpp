#pragma once

#include <cerrno>
#include <cstring>
#include <exception>
#include <sstream>

#include "../../Typedef.hpp"


class ConnectionException : public std::exception {
protected:
	String	_err;


protected:
	String	generateSysCallErrorMessage(const String& sys_call) {
		std::stringstream ss;

		// fetch the current time
		time_t rawtime;
		time(&rawtime);

		// convert to local time format in the tm structure
		struct tm* timeinfo;
		timeinfo = localtime(&rawtime);

		// format: YYYY-MM-DD HH:MM:SS
		char buffer[80];
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

		ss << "[" << buffer << "] "<< "System call Error: "
		   << sys_call << " failed. Reason: " << strerror(errno);

		return ss.str();
	}

public:

	ConnectionException() {}

	ConnectionException(const String& sys_call) {
		this->_err = generateSysCallErrorMessage(sys_call);
	}

	const char * what() const throw() {
		return this->_err.c_str();
	}

	~ConnectionException() throw() {}
};
