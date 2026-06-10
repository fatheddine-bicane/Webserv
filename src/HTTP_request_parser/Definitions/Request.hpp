#pragma once

#include <map>

#include "../../Includes/Typedef.hpp"


enum RequestState {
	// parse ongoing
	START_LINE, HEADERS, BODY,
};


enum HTTPMethod {
	GET, POST, DELETE, PUT, UNSUPPORTED
};


// INFO: main class
class Request {
public:
	RequestState				state;

	// HTTP message
	// start-line
	HTTPMethod					method;
	String						target;
	String						HTTP_version;
	// headers
	std::map<String, String>	headers;
	// TODO:body handling to be determined


public:
	// INFO: constructor
	Request();
};
