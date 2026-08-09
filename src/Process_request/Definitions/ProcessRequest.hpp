#pragma once

#include "../../HTTP_request_parser/Definitions/Request.hpp"
#include <cstddef>
#include <iterator>
#include <vector>


class ProcessRequest {
private:
	Request&	_request;
	Server&		_server;
	Location*	_location;

	// cgi
	String		_interpreter;
	size_t		_extention_pos;


public:
	// INFO: constructor
	ProcessRequest(Request& request, Server& server);


public:
	// INFO: api
	void	processRequest();
	// send response


private:
	// cgi
	void	processCGIRequest();
	void	setPathEnvVariables(std::vector<String>& env);
	void	setHeadersEnvVariables(std::vector<String>& env);
	bool	isCGIRequest();



	void	processGetRequest();
	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();

};
