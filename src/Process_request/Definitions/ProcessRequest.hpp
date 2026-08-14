#pragma once

#include "../../HTTP_request_parser/Definitions/Request.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include <cstddef>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/epoll.h>
#include <sys/wait.h>





class ProcessRequest {

private:
	enum CGIState {
		READING_HEADERS, READING_BODY
	};


private:
	Request&	_request;
	Server&		_server;
	ClientConnection&	_client_connection;


public:
	// INFO: constructor
	ProcessRequest(Request& request, Server& server,
				   ClientConnection& client_connection);


public:
	// INFO: api
	void	processRequest();
	// send response

	// cgi processors
	void	monitoreCGIPipe(EP_INSTANCE epfd);
	void	readCGIPipe(EP_INSTANCE epfd);
	void	parseCGIHeaders();
	bool	isCGISucceed(std::vector<pid_t>& cgis_to_reap);

private:
	void	processGetRequest();
	void	renderDirectoryListing();

	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();

};
