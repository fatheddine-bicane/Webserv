#pragma once

#include "../../HTTP_request_parser/Definitions/Request.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include <cstddef>
#include <iterator>
#include <vector>
#include <sys/epoll.h>
#include <sys/wait.h>


class ProcessRequest {
private:
	Request&	_request;
	Server&		_server;
	Location*	_location;
	ClientConnection&	_client_connection;

	// cgi
	String		_interpreter;
	size_t		_extention_pos;
	EP_INSTANCE	_epfd;


	String		_file_path;

public:
	// INFO: constructor
	ProcessRequest(Request& request, Server& server,
				   ClientConnection& client_connection);


public:
	// INFO: api
	void	processRequest();
	// send response


private:
	String	getFilePath();
	void	resolveFilePath();



	// cgi
	void	processCGIRequest();
	void	setPathEnvVariables(std::vector<String>& env);
	void	setHeadersEnvVariables(std::vector<String>& env);
	void	setUpChildProcess(PIPE& fds, std::vector<String>& env);
	void	setUpParentProcess(PIPE& fds);
	bool	isCGIRequest();



	void	processGetRequest();
	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();

};
