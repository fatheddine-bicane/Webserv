#pragma once

#include "../../HTTP_request_parser/Definitions/Request.hpp"


class ProcessRequest {
private:
	Request& _request;


public:
	// INFO: constructor
	ProcessRequest(Request& request);


public:
	// INFO: api
	void	processRequest();
	// send response


private:
	void	processGetRequest();
	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();
};
