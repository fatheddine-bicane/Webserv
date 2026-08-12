#pragma once

#include "../../HTTP_request_parser/Definitions/Request.hpp"


class ProcessRequest {
private:
	Request&	_request;
	Server&		_server;
	Location*	_location;

	String		_file_path;
public:
	// INFO: constructor
	ProcessRequest(Request& request, Server& server);


public:
	// INFO: api
	void	processRequest();
	// send response


private:
	void	resolveFilePath();

	// get request helpers
	void	processGetRequest();
	void	renderDirectoryListing();

	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();

};
